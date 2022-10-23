/**
 * @file Hist.hpp Simple 1D histogram implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_HIST_ALGS_HPP_
#define DQM_INCLUDE_DQM_HIST_ALGS_HPP_

// DQM
#include <vector>

/**
 * Basic 1D histogram that counts entries
 * It only supports uniform binning
 * Overflow and underflow is not supported yet
 */
namespace dunedaq::dqm {

class Hist
{

  int find_bin(double x) const;

public:
  double m_low, m_high, m_step_size;
  int m_nentries = 0;

  int m_steps;
  std::vector<int> m_entries;

  /**
   * @brief Hist constructor
   * @param steps Number of uniform bins
   * @param low Lower limit of the histogram
   * @param high Upper limit of the histogram
   */
  Hist(int steps, double low, double high);

  /**
   * @brief Add an entry to the histogram
   * @param x The number that is being added
   */
  int fill(double x);

  bool is_running() const;
  void clean();

  double mean();
  double std();
};

Hist::Hist(int steps, double low, double high)
  : m_low(low)
  , m_high(high)
  , m_steps(steps)
{
  m_entries = std::vector<int>(steps, 0);
  m_step_size = (high - low) / steps;
}

int
Hist::find_bin(double x) const
{
  return (x - m_low) / m_step_size;
}

int
Hist::fill(double x)
{
  int bin = find_bin(x);
  // Underflow, do nothing
  if (bin < 0) {
    return -1;
  }

  // Overflow, do nothing
  if (bin >= m_steps) {
    return -1;
  }

  m_entries[bin]++;
  m_nentries++;
  return bin;
}

bool
Hist::is_running() const
{
  return true;
}

void
Hist::clean()
{
  m_nentries = 0;
  for (auto& elem : m_entries) {
    elem = 0;
  }
}

} // namespace dunedaq::dqm

#endif // DQM_INCLUDE_DQM_HIST_ALGS_HPP_
