/**
 * @file Fourier.hpp Fast fourier transforms using the fftw3 library
 *
 * This is part of the DUNE DAQ, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_FOURIER_HPP_
#define DQM_INCLUDE_DQM_FOURIER_HPP_

// dqm
#include "AnalysisModule.hpp"
#include "Decoder.hpp"

#include "daqdataformats/TriggerRecord.hpp"
#include "logging/Logging.hpp"

#include <complex>
#include <string>
#include <valarray>
#include <vector>

// #include <complex> has to be before this include
#include <fftw3.h>

namespace dunedaq::dqm {

typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;

class Fourier
{
public:
  double m_inc_size;
  int m_npoints;
  std::vector<double> m_data;
  std::vector<double> m_transform;

  Fourier(double inc, int npoints);

  void fill(double value);
  CArray compute_fourier();
  void compute_fourier_normalized();
  CArray compute_fourier_def();
  void compute_fourier_transform(fftw_plan &plan);
  std::vector<double> get_frequencies();
  void clean();

  double get_transform_at(int index);
  std::vector<double> get_transform();
};

Fourier::Fourier(double inc, int npoints) // NOLINT(build/unsigned)
  : m_inc_size(inc)
  , m_npoints(npoints)
{
  m_data.reserve(npoints);
  m_transform = std::vector<double> (npoints);
}

CArray
Fourier::compute_fourier_def()
{
  // if (m_data.size() != m_npoints)
  //  TLOG() << "The number of points in the data is different from the number of points specified, m_data.size() = " <<
  //  m_data.size() << " and m_npoints = " << m_npoints;
  CArray output = CArray(0.0, m_data.size() / 2);

  // Only until m_data.size() / 2, the others correspond to the negative frequencies
  for (size_t k = 0; k < m_data.size() / 2; ++k)
    for (size_t m = 0; m < m_data.size(); ++m) {
      output[k] += m_data[m] * std::exp(Complex(0, -2) * M_PI * static_cast<double>(k) * static_cast<double>(m) /
                                        static_cast<double>(m_data.size()));
    }
  return output;
}

void
Fourier::compute_fourier_normalized()
{
  CArray output = compute_fourier_def();

  // Only until m_data.size() / 2, the others correspond to the negative frequencies
  for (size_t k = 0; k < m_data.size() / 2; ++k) {
    output[k] = 2. / m_npoints * abs(output[k]);
    m_transform.push_back(abs(output[k]));
  }
}


/**
 * @brief Compute the absolute value of the fourier transform
 *        using the FFTW library
 */
void
Fourier::compute_fourier_transform(fftw_plan &plan) {
  fftw_execute_r2r(plan, m_data.data(), m_transform.data());
  // After the transform is computed half of the elements of the
  // output array are the real part and the other half are the
  // complex part, this computes the absolute value of each value

  // Caveats, i = 0 and i = m_npoints/2 are already real and i = 0 is already
  // positive so only i = m_npoints/2 has to be changed
  for (int i = 1; i < m_npoints / 2; ++i) {
    m_transform[i] = sqrt(m_transform[i] * m_transform[i] +
                          m_transform[m_npoints - i] * m_transform[m_npoints - i]);
  }
  m_transform[m_npoints / 2] = abs(m_transform[m_npoints / 2]);
  m_transform.resize(m_npoints / 2 + 1);
}


std::vector<double>
Fourier::get_transform() {
  return m_transform;
}

/**
 * @brief Get the absolute value of the fourier transform at index index
 *
 *        Must be called after compute_fourier_transform
 */
double
Fourier::get_transform_at(int index)
{
  if (index < 0 || static_cast<size_t>(index) >= m_transform.size()) {
    TLOG() << "WARNING: Fourier::get_transform called with index out of range, index=" << index
           << ", size of m_transform vector is " << m_transform.size();
    return 0.0;
  }
  return m_transform[index];
}

/**
 * @brief Get the output frequencies (only non-negative frequencies)
 */
std::vector<double>
Fourier::get_frequencies()
{
  std::vector<double> ret;
  for (int i = 0; i <= m_npoints / 2; ++i)
    ret.push_back(i / (m_inc_size * m_npoints));
  // Don't return the negative frequencies by commenting the following block
  // for (int i = -m_npoints/2; i < 0; ++i)
  //   ret.push_back(i / (m_inc_size * m_npoints));
  return ret;
}

/**
 * @brief Fill the values that will be used to compute the fourier transform
 *
 */
void
Fourier::fill(double value) // NOLINT(build/unsigned)
{
  m_data.push_back(value);
}

/**
 * @brief Clear the vectors that hold the data and the fourier transform
 *
 *        This function leaves the Fourier object not usable anymore
 */
void
Fourier::clean()
{
  m_data.clear();
  m_transform.clear();
}

} // namespace dunedaq::dqm

#endif // DQM_INCLUDE_DQM_FOURIER_HPP_
