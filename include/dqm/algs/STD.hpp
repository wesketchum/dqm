/**
 * @file STD.hpp Simple Standard Deviation implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_ALGS_STD_HPP_
#define DQM_INCLUDE_DQM_ALGS_STD_HPP_

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
  void fill(const double x);

  void clean();

  double std() const;
};

} // namespace dunedaq
} // namespace dqm

#endif // DQM_INCLUDE_DQM_ALGS_STD_HPP_
