/**
 * @file RMS.hpp Declarations for a simple RMS implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_ALGS_RMS_HPP_
#define DQM_INCLUDE_DQM_ALGS_RMS_HPP_

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

  void clean();

  double rms() const;
};

} // namespace dqm
} // namespace dunedaq

#endif // DQM_INCLUDE_DQM_ALGS_RMS_HPP_
