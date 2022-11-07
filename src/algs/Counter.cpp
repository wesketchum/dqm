/**
 * @file Counter.cxx Simple counter implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_ALGS_COUNTER_CXX_
#define DQM_SRC_ALGS_COUNTER_CXX_

#include "dqm/algs/Counter.hpp"

/**
 * Basic Counter implementation
 * Fill values and store them
 */
namespace dunedaq {
namespace dqm {

void
Counter::fill(double const x)
{
  m_nentries++;
  count.push_back(x);
}

void
Counter::clean()
{
  m_nentries = 0;
  count.clear();
}

} // namespace dunedaq
} // namespace dqm

#endif // DQM_SRC_ALGS_COUNTER_CXX_
