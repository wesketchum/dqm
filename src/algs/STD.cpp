/**
 * @file STD.cpp Simple Standard Deviation implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_DQM_ALGS_STD_CPP_
#define DQM_SRC_DQM_ALGS_STD_CPP_

#include "dqm/algs/STD.hpp"
#include <cmath>

/**
 * Basic STD calculation implementation
 * Fill values, then compute the STD at the end
 */
namespace dunedaq {
namespace dqm {

void
STD::fill(double const x)
{
  m_nentries++;
  m_sum += x;
  m_sum_sq += x * x;
}

void
STD::clean()
{
  m_sum = 0;
  m_sum_sq = 0;
  m_nentries = 0;
}

double
STD::std() const
{
  if (m_std_set) {
    return m_std;
  }
  if (m_nentries <= 1) {
    return -1;
  }
  auto mean = m_sum / m_nentries;
  return sqrt((m_sum_sq + m_nentries * mean * mean - 2 * m_sum * mean) / (m_nentries - 1));
}

} // namespace dunedaq
} // namespace dqm

#endif // DQM_SRC_DQM_ALGS_STD_CPP_
