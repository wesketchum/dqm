/**
 * @file RMS.hpp Simple RMS implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_RMS_HPP_
#define DQM_INCLUDE_DQM_RMS_HPP_

#include <cmath>

/**
 * Basic RMS calculation implementation
 * Fill values, then compute the RMS at the end
 */
namespace dunedaq {
namespace dqm {

class RMS
{

public:
  int m_nentries = 0;
  double m_sum_sq = 0, m_rms;
  bool m_rms_set = false;

  /**
   * @brief Add an entry to the histogram
   * @param x The number that is being added
   */
  void fill(double const x);

  bool is_running() const;
  void clean();

  double mean();
  double rms();
};

void
RMS::fill(double const x)
{
  m_nentries++;
  m_sum_sq += x * x;
}

bool
RMS::is_running() const
{
  return true;
}

void
RMS::clean()
{
  m_sum_sq = 0;
  m_nentries = 0;
  m_rms_set = false;
}

double
RMS::rms()
{
  if (m_rms_set)
    return m_rms;
  if (not m_nentries) {
    return -1;
  }
  m_rms = sqrt(m_sum_sq / m_nentries);
  m_rms_set = true;
  return m_rms;
}

} // namespace dqm
} // namespace dunedaq

#endif // DQM_INCLUDE_DQM_RMS_HPP_
