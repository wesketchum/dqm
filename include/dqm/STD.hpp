/**
 * @file STD.hpp Simple Standard Deviation implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_STDEV_HPP_
#define DQM_INCLUDE_DQM_STDEV_HPP_

#include <cmath>

/**
 * Basic STD calculation implementation
 * Fill values, then compute the STD at the end
 */
namespace dunedaq {
namespace dqm {

class STD
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
STD::fill(double const x)
{
  m_nentries++;
  m_sum += x;
  m_sum_sq += x * x;
}

bool
STD::is_running() const
{
  return true;
}

void
STD::clean()
{
  m_sum = 0;
  m_sum_sq = 0;
  m_nentries = 0;
  m_std_set = false;
  m_mean_set = false;
}

double
STD::mean()
{
  if (m_mean_set) {
    return m_mean;
  }
  m_mean = m_sum / m_nentries;
  m_mean_set = true;
  return m_mean;
}

double
STD::std()
{
  if (m_std_set) {
    return m_std;
  }
  if (m_nentries <= 1) {
    return -1;
  }
  m_mean = mean();
  m_std = sqrt((m_sum_sq + m_nentries * m_mean * m_mean - 2 * m_sum * m_mean) / (m_nentries - 1));
  m_std_set = true;
  return m_std;
}

} // namespace dunedaq
} // namespace dqm

#endif // DQM_INCLUDE_DQM_STDEV_HPP_
