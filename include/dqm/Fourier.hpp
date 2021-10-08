/**
 * @file Fourier.hpp Fast fourier transform using the Cooley-Tukey algorithm
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_FOURIER_HPP_
#define DQM_SRC_FOURIER_HPP_

// dqm
#include "AnalysisModule.hpp"
#include "Decoder.hpp"

#include "dataformats/TriggerRecord.hpp"
#include "logging/Logging.hpp"

#include <complex>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <valarray>
#include <vector>

namespace dunedaq::dqm {

typedef std::complex<double> Complex;
typedef std::valarray<Complex> CArray;


class Fourier
{
public:

  CArray fourier_prep(const std::vector<double>& input) const;
  void fast_fourier_transform(CArray& x);

  double m_inc_size;
  int m_npoints;
  std::vector<double> m_data;
  std::vector<double> m_transform;

  Fourier(double inc, int npoints);

  void fill(double value);
  CArray compute_fourier();
  void compute_fourier_normalized();
  CArray compute_fourier_def();
  std::vector<double> get_frequencies();
  void clean();

  double get_transform(int index);

  // void save(const std::string& filename) const;
  // void save(std::ofstream& filehandle) const;

  // void save_fourier(const std::string& filename) const;
  // void save_fourier(std::ofstream& filehandle) const;
};

Fourier::Fourier(double inc, int npoints) // NOLINT(build/unsigned)
  : m_inc_size(inc)
  , m_npoints(npoints)
{
  m_data.reserve(m_npoints);
}

CArray
Fourier::fourier_prep(const std::vector<double>& input) const
{
  CArray output(input.size());
  for (size_t i = 0; i < input.size(); i++) {
    output[i] = input[i];
    if (i < 100)
      std::cout << "Prep " << output[i] << " " << input[i] << std::endl;
  }
  return output;
}

// CArray
// Fourier::fourier_rebin(CArray input, double factor)
// {
//   // Unused
//   int newsize = static_cast<int>(input.size() / factor);
//   std::valarray<Complex> output(newsize);
//   for (size_t i = 0; i < input.size(); i++) {
//     int k = static_cast<int>(i) / factor;
//     // std::cout << "i = " << i << ", k = " << k << std::endl;
//     output[k] += input[i];
//   }
//   return output;
// }

// Cooley-Tukey FFT (in-place, breadth-first, decimation-in-frequency)
// void
// Fourier::fast_fourier_transform(CArray& x)
// {
//   // DFT
//   unsigned int N = x.size(), k = N, n;
//   double thetaT = 3.14159265358979323846264338328L / N;
//   Complex phiT = Complex(cos(thetaT), -sin(thetaT)), T;
//   while (k > 1) {
//     n = k;
//     k >>= 1;
//     phiT = phiT * phiT;
//     T = 1.0L;
//     for (unsigned int l = 0; l < k; l++) {
//       for (unsigned int a = l; a < N; a += n) {
//         unsigned int b = a + k;
//         Complex t = x[a] - x[b];
//         x[a] += x[b];
//         x[b] = t * T;
//       }
//       T *= phiT;
//     }
//   }
//   std::cout << "First part done" << std::endl;
//   // Decimate
//   unsigned int m = (unsigned int)log2(N);
//   for (unsigned int a = 0; a < N; a++) {
//     unsigned int b = a;
//     // Reverse bits
//     b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
//     b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
//     b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
//     b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
//     b = ((b >> 16) | (b << 16)) >> (32 - m);
//     if (b > a) {
//       Complex t = x[a];
//       x[a] = x[b];
//       x[b] = t;
//     }
//   }
//   std::cout << "Second part done" << std::endl;
// }

void
Fourier::fast_fourier_transform(CArray& x)
{
    const size_t N = x.size();
    if (N <= 1) return;

    // divide
    CArray even = x[std::slice(0, N/2, 2)];
    CArray  odd = x[std::slice(1, N/2, 2)];

    // conquer
    fast_fourier_transform(even);
    fast_fourier_transform(odd);

    // combine
    for (size_t k = 0; k < N/2; ++k)
    {
        Complex t = std::polar(1.0, -2 * M_PI * k / N) * odd[k];
        x[k    ] = even[k] + t;
        x[k+N/2] = even[k] - t;
    }
}

CArray
Fourier::compute_fourier()
{
  // std::cout << "Computing FT" << std::endl;
  CArray input = fourier_prep(m_data);
  std::cout << "Input prepared" << std::endl;
  fast_fourier_transform(input);
  // m_data.clear();
  std::cout << "Transform performed" << std::endl;
  // Unused
  // int newsize = (int) input.size()/rebin_factor;
  // std::cout << "Size of array after rebinning = " << newsize << std::endl;
  return input;
  // CArray out_array(input);
  // std::cout << "CArray set up" << std::endl;
  // out_array = fourier_rebin(input, rebin_factor);
  // std::cout << "Rebinning complete" << std::endl;
  // m_rebin_factor = rebin_factor;
  // std::cout << "Rebin factor saved" << std::endl;
  // m_fourier_transform.resize(input.size());
  // std::cout << "Beginning loop" << std::endl;
  // for (size_t i = 0; i < out_array.size(); i++) {
  //   // std::cout << "i = " << i << std::endl;
  //   double val = static_cast<double>(out_array[i].real());
  //   // std::cout << "val = " << val << std::endl;
  //   m_fourier_transform[i] = val;
  //   // std::cout << "Pushed back" << std::endl;
  // }
  // std::cout << "Completing" << std::endl;
}

CArray
Fourier::compute_fourier_def()
{
  if (m_data.size() != m_npoints)
    TLOG() << "The number of points in the data is different from the number of points specified";
  CArray output = CArray(0.0, m_data.size() / 2);

  // Only until m_data.size() / 2, the others correspond to the negative frequencies
  for (size_t k = 0; k < m_data.size() / 2; ++k)
    for (size_t m = 0; m < m_data.size(); ++m) {
      output[k] += m_data[m] * std::exp( Complex(0, -2) * M_PI * static_cast<double>(k) * static_cast<double>(m) / static_cast<double>(m_data.size()));
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
  // std::stringstream ss;
  // for (auto& elem: m_data)
  //   ss << elem << " ";
  // TLOG() << "Input: " << ss.str();
  // std::stringstream sso;
  // for (auto& elem: output)
  //   sso << abs(elem) << " ";
  // TLOG() << "Output: " << sso.str();
  // return output;

}

double
Fourier::get_transform(int index)
{
  return m_transform[index];
}


std::vector<double>
Fourier::get_frequencies()
{
  std::vector<double> ret;
  for (int i = 0; i < m_npoints/2; ++i)
    ret.push_back(i / (m_inc_size * m_npoints));
  // Don't return the negative ones
  // for (int i = -m_npoints/2; i < 0; ++i)
  //   ret.push_back(i / (m_inc_size * m_npoints));
  return ret;
}

void
Fourier::fill(double value) // NOLINT(build/unsigned)
{
  m_data.push_back(value);
}
void
Fourier::clean()
{
  m_data.clear();
  m_transform.clear();
}

// void
// Fourier::save(const std::string& filename) const
// {
//   std::ofstream file;
//   file.open(filename);
//   file << m_start << " " << m_end << " " << m_inc_size << " " << m_npoints << " " << std::endl;
//   for (auto x : m_data) {
//     file << x << " ";
//   }
//   file << std::endl;
// }

// void
// Fourier::save(std::ofstream& filehandle) const
// {
//   filehandle << m_start << " " << m_end << " " << m_inc_size << " " << m_npoints << " " << std::endl;
//   for (auto x : m_data) {
//     filehandle << x << " ";
//   }
//   filehandle << std::endl;
// }

// void
// Fourier::save_fourier(const std::string& filename) const
// {
//   std::ofstream file;
//   file.open(filename);
//   file << m_start << " " << m_end << " " << m_inc_size << " " << m_npoints << " " << m_rebin_factor << std::endl;
//   for (auto x : m_fourier_transform) {
//     file << x << " ";
//   }
//   file << std::endl;
// }

// void
// Fourier::save_fourier(std::ofstream& filehandle) const
// {
//   filehandle << m_start << " " << m_end << " " << m_inc_size << " " << m_npoints << " " << m_rebin_factor << std::endl;
//   for (auto x : m_fourier_transform) {
//     filehandle << x << " ";
//   }
//   filehandle << std::endl;
// }


} // namespace dunedaq::dqm

#endif // DQM_SRC_FOURIER_HPP_
