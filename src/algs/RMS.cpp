/**
 * @file RMS.cpp Simple RMS implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_DQM_ALGS_RMS_CPP_
#define DQM_SRC_DQM_ALGS_RMS_CPP_

#include "dqm/algs/RMS.hpp"
#include <cmath>

/**
 * Basic RMS calculation implementation
 * Fill values, then compute the RMS at the end
 */
namespace dunedaq {
namespace dqm {

void
RMS::fill(const double x)
{
  m_nentries++;
  m_sum_sq += x * x;
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

#endif // DQM_SRC_DQM_ALGS_RMS_CPP_
