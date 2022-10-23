/**
 * @file Fourier.hpp Declarations for fast fourier transforms using the fftw3 library
 *
 * This is part of the DUNE DAQ, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_ALGS_FOURIER_HPP_
#define DQM_INCLUDE_DQM_ALGS_FOURIER_HPP_


#include <vector>
#include <complex>
// #include <complex> has to be before this include
#include <fftw3.h>

#include "dqm/Issues.hpp"
#include "logging/Logging.hpp"

namespace dunedaq::dqm {

class Fourier
{
public:
  double m_inc_size;
  int m_npoints;
  std::vector<double> m_data;
  std::vector<double> m_transform;

  Fourier(double inc, int npoints);

  void fill(double value);
  void compute_fourier_transform();
  std::vector<double> get_frequencies();
  void clean();

  double get_transform_at(int index);
  std::vector<double> get_transform();
};


} // namespace dunedaq::dqm

#endif // DQM_INCLUDE_DQM_ALGS_FOURIER_HPP_
