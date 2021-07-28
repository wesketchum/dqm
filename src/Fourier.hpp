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

  double find_value(uint64_t time); // NOLINT(build/unsigned)
  CArray fourier_prep(const std::vector<double>& input) const;
  CArray fourier_rebin(CArray input, double factor);
  void fast_fourier_transform(CArray& x);

public:
  uint64_t m_start;    // NOLINT(build/unsigned)
  uint64_t m_end;      // NOLINT(build/unsigned)
  uint64_t m_inc_size; // NOLINT(build/unsigned)
  int m_npoints;
  std::vector<double> m_data;
  double m_rebin_factor;
  // CArray m_fourier_transform (10000);
  std::vector<double> m_fourier_transform;

  Fourier(uint64_t start, uint64_t end, int npoints); // NOLINT(build/unsigned)
  int enter(double value, uint64_t time);             // NOLINT(build/unsigned)
  void compute_fourier(double rebin_factor);

  void save(const std::string& filename) const;
  void save(std::ofstream& filehandle) const;

  void save_fourier(const std::string& filename) const;
  void save_fourier(std::ofstream& filehandle) const;
};

Fourier::Fourier(uint64_t start, uint64_t end, int npoints) // NOLINT(build/unsigned)
//  : m_start(start), m_end(end), m_npoints(npoints)
{
  m_start = start;
  m_end = end;
  m_npoints = npoints;

  // Unused
  // uint64_t npoints64 = (uint64_t) npoints; // NOLINT(build/unsigned)
  // m_inc_size = (end - start)/npoints64;
  m_inc_size = 25;
  m_data = std::vector<double>(npoints, 0);
}

CArray
Fourier::fourier_prep(const std::vector<double>& input) const
{
  std::valarray<Complex> output(input.size());
  Complex val;
  for (size_t i = 0; i < input.size(); i++) {
    val = input[i];
    output[i] = val;
  }
  return output;
}

CArray
Fourier::fourier_rebin(CArray input, double factor)
{
  // Unused
  int newsize = static_cast<int>(input.size() / factor);
  std::valarray<Complex> output(newsize);
  for (size_t i = 0; i < input.size(); i++) {
    int k = static_cast<int>(i) / factor;
    // std::cout << "i = " << i << ", k = " << k << std::endl;
    output[k] += input[i];
  }
  return output;
}

// Cooley-Tukey FFT (in-place, breadth-first, decimation-in-frequency)
void
Fourier::fast_fourier_transform(CArray& x)
{
  // DFT
  unsigned int N = x.size(), k = N, n;
  double thetaT = 3.14159265358979323846264338328L / N;
  Complex phiT = Complex(cos(thetaT), -sin(thetaT)), T;
  while (k > 1) {
    n = k;
    k >>= 1;
    phiT = phiT * phiT;
    T = 1.0L;
    for (unsigned int l = 0; l < k; l++) {
      for (unsigned int a = l; a < N; a += n) {
        unsigned int b = a + k;
        Complex t = x[a] - x[b];
        x[a] += x[b];
        x[b] = t * T;
      }
      T *= phiT;
    }
  }
  // Decimate
  unsigned int m = (unsigned int)log2(N);
  for (unsigned int a = 0; a < N; a++) {
    unsigned int b = a;
    // Reverse bits
    b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
    b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
    b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
    b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
    b = ((b >> 16) | (b << 16)) >> (32 - m);
    if (b > a) {
      Complex t = x[a];
      x[a] = x[b];
      x[b] = t;
    }
  }
}

void
Fourier::compute_fourier(double rebin_factor)
{
  // std::cout << "Computing FT" << std::endl;
  CArray input = fourier_prep(m_data);
  // std::cout << "Input prepared" << std::endl;
  fast_fourier_transform(input);
  // m_data.clear();
  // std::cout << "Transform performed" << std::endl;
  // Unused
  // int newsize = (int) input.size()/rebin_factor;
  // std::cout << "Size of array after rebinning = " << newsize << std::endl;
  CArray out_array(input);
  // std::cout << "CArray set up" << std::endl;
  // out_array = fourier_rebin(input, rebin_factor);
  // std::cout << "Rebinning complete" << std::endl;
  m_rebin_factor = rebin_factor;
  // std::cout << "Rebin factor saved" << std::endl;
  m_fourier_transform.resize(input.size());
  // std::cout << "Beginning loop" << std::endl;
  for (size_t i = 0; i < out_array.size(); i++) {
    // std::cout << "i = " << i << std::endl;
    double val = static_cast<double>(out_array[i].real());
    // std::cout << "val = " << val << std::endl;
    m_fourier_transform[i] = val;
    // std::cout << "Pushed back" << std::endl;
  }
  // std::cout << "Completing" << std::endl;
}

int
Fourier::enter(double value, uint64_t time) // NOLINT(build/unsigned)
{
  uint64_t index64 = (time - m_start) / m_inc_size; // NOLINT(build/unsigned)
  if (index64 > static_cast<uint64_t>(m_npoints))   // NOLINT(build/unsigned)
  {
    TLOG() << "-------OVERSPILL---------";
    TLOG() << "index = ( time (" << time << ") - m_start (" << m_start << ") ) / m_inc_size (" << m_inc_size
           << ") = " << index64 << ", should be less than " << m_npoints;
    TLOG() << "Curr. time = " << time << ", should be less than end time = " << m_end
           << ". Difference = " << m_end - time;
    TLOG() << "m_inc_size = ( end (" << m_end << ") - start (" << m_start << ") ) / npoints (" << m_npoints << ")";
  }

  int index = static_cast<int>(index64);
  if (index > m_npoints) {
    TLOG() << "Cannot enter a time that lies outside scope of series.";
    return -1;
  } else {
    m_data[index] = value;
  }

  return 1;
}

double
Fourier::find_value(uint64_t time) // NOLINT(build/unsigned)
{
  uint64_t index64 = (time - m_start) / m_inc_size; // NOLINT(build/unsigned)
  int index = static_cast<int>(index64);
  if ((index > m_npoints) || (index < 0)) {
    TLOG() << "Cannot find a time that lies outside scope of series.";
    return -9999;
  } else {
    return m_data[index];
  }
}

void
Fourier::save(const std::string& filename) const
{
  std::ofstream file;
  file.open(filename);
  file << m_start << " " << m_end << " " << m_inc_size << " " << m_npoints << " " << std::endl;
  for (auto x : m_data) {
    file << x << " ";
  }
  file << std::endl;
}

void
Fourier::save(std::ofstream& filehandle) const
{
  filehandle << m_start << " " << m_end << " " << m_inc_size << " " << m_npoints << " " << std::endl;
  for (auto x : m_data) {
    filehandle << x << " ";
  }
  filehandle << std::endl;
}

void
Fourier::save_fourier(const std::string& filename) const
{
  std::ofstream file;
  file.open(filename);
  file << m_start << " " << m_end << " " << m_inc_size << " " << m_npoints << " " << m_rebin_factor << std::endl;
  for (auto x : m_fourier_transform) {
    file << x << " ";
  }
  file << std::endl;
}

void
Fourier::save_fourier(std::ofstream& filehandle) const
{
  filehandle << m_start << " " << m_end << " " << m_inc_size << " " << m_npoints << " " << m_rebin_factor << std::endl;
  for (auto x : m_fourier_transform) {
    filehandle << x << " ";
  }
  filehandle << std::endl;
}

class FourierLink : public AnalysisModule
{
  std::string m_name;
  std::vector<Fourier> fouriervec;
  bool m_run_mark;

public:
  FourierLink(std::string name, int start, int end, int npoints);

  void run(dunedaq::dataformats::TriggerRecord& tr, RunningMode mode, std::string kafka_address);
  bool is_running();
};

FourierLink::FourierLink(std::string name, int start, int end, int npoints)
  : m_name(name)
  , m_run_mark(false)
{
  for (int i = 0; i < 256; ++i) {
    Fourier fourier(start, end, npoints);
    fouriervec.push_back(fourier);
  }
}

void
FourierLink::run(dunedaq::dataformats::TriggerRecord& tr, RunningMode, std::string kafka_address)
{
  m_run_mark = true;
  dunedaq::dqm::Decoder dec;
  auto wibframes = dec.decode(tr);

  for (auto fr : wibframes) {
    for (int ich = 0; ich < 256; ++ich) {
      fouriervec[ich].enter(fr->get_channel(ich), 0);
    }
  }

  for (int ich = 0; ich < 256; ++ich) {
    fouriervec[ich].save("Fourier/" + m_name + "-" + std::to_string(ich) + ".txt");
  }
  m_run_mark = false;
}

bool
FourierLink::is_running()
{
  return m_run_mark;
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_FOURIER_HPP_
