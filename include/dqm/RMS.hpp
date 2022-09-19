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
  double m_sum_sq = 0;

  /**
   * @brief Add an entry to the histogram
   * @param x The number that is being added
   */
  void fill(const double x);

  bool is_running() const;
  void clean();

  double rms() const;
};

void
RMS::fill(const double x)
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
}

double
RMS::rms() const
{
  if (not m_nentries) {
    return -1;
  }
  return sqrt(m_sum_sq / m_nentries);

}

} // namespace dqm
} // namespace dunedaq

#endif // DQM_INCLUDE_DQM_RMS_HPP_
