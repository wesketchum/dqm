/**
 * @file Counter.hpp Simple counter declaration
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_COUNTER_HPP_
#define DQM_INCLUDE_DQM_COUNTER_HPP_

#include <vector>

/**
 * Basic Counter implementation
 * Fill values and store them
 */
namespace dunedaq {
namespace dqm {

class Counter
{

public:
  std::vector<int> count;
  int m_nentries = 0;

  /**
   * @brief Add an entry to the counts
   * @param x The number that is being added
   */
  void fill(const double x);

  void clean();
};

} // namespace dunedaq
} // namespace dqm

#endif // DQM_INCLUDE_DQM_COUNTER_HPP_
