/**
 * @file StDev.hpp Simple 1D histogram implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_STDEV_HPP_
#define DQM_INCLUDE_DQM_STDEV_HPP_

#include "AnalysisModule.hpp"

#include <cmath>

/**
 * Basic StDev calculation implementation
 * Fill values, then compute the StDev at the end
 */
namespace dunedaq::dqm {

class StDev
{

public:
  int m_nentries = 0;
  double m_sum = 0, m_sum_sq = 0, m_mean, m_std;
  bool m_mean_set = false, m_std_set = false;

  /**
   * @brief Add an entry to the histogram
   * @param x The number that is being added
   */
  void fill(double const x);

  bool is_running() const;
  void clean();

  double mean();
  double std();
};


void
StDev::fill(double const x)
{
  m_nentries++;
  m_sum += x;
  m_sum_sq += x * x;
}

bool
StDev::is_running() const
{
  return true;
}

void
StDev::clean()
{
  m_sum = 0;
  m_sum_sq = 0;
  m_nentries = 0;
  m_std_set = false;
  m_mean_set = false;
}

double
StDev::mean()
{
  if (m_mean_set)
    return m_mean;
  m_mean = m_sum / m_nentries;
  m_mean_set = true;
  return m_mean;
}

double
StDev::std()
{
  if (m_std_set)
    return m_std;
  if (m_nentries <= 1) {
    ers::error(InvalidInput(ERS_HERE, "number of entries is 0 or 1 for the StDev"));
    return -1;
  }
  m_mean = mean();
  m_std = sqrt((m_sum_sq + m_nentries * m_mean * m_mean - 2 * m_sum * m_mean) / (m_nentries - 1));
  m_std_set = true;
  return m_std;
}

} // namespace dunedaq::dqm

#endif // DQM_INCLUDE_DQM_STDEV_HPP_
