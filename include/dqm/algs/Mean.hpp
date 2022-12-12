/**
 * @file Mean.hpp Simple mean implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_MEAN_HPP_
#define DQM_INCLUDE_DQM_MEAN_HPP_

// DQM

/**
 * Basic 1D histogram that counts entries
 * It only supports uniform binning
 * Overflow and underflow is not supported yet
 */
namespace dunedaq::dqm {

class Mean
{

public:
  int m_nentries = 0;
  double m_sum = 0;

  void fill(double x);

  void clean();

  double mean();
};

void
Mean::fill(double x)
{
  m_nentries++;
  m_sum += x;
}

void
Mean::clean()
{
  m_sum = 0;
  m_nentries = 0;
}

double
Mean::mean()
{
  m_mean = m_sum / m_nentries;
  return m_mean;
}

} // namespace dunedaq::dqm

#endif // DQM_INCLUDE_DQM_MEAN_HPP_
