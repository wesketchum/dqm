/**
 * @file Hist.hpp Simple 1D histogram implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_HIST_HPP_
#define DQM_SRC_HIST_HPP_

// DQM
#include "AnalysisModule.hpp"
#include "Decoder.hpp"
#include "Exporter.hpp"

#include "dataformats/TriggerRecord.hpp"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
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
  int m_nentries;
  double m_sum;

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

  /**
   * @brief Save to a text file (for debugging purposes)
   * @param filename Name of the file where the histogram will be saved
   */
  void save(const std::string& filename) const;

  /**
   * @brief Save to a text file (for debugging purposes)
   * @param filehandle Handle of the file where the histogram will be saved
   */
  void save(std::ofstream& filehandle) const;

  bool is_running();
  void run(dunedaq::dataformats::TriggerRecord& tr);
  void clean();
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
  m_sum += x;
  return bin;
}

void
Hist::save(const std::string& filename) const
{
  std::ofstream file;
  file.open(filename);
  file << m_steps << " " << m_low << " " << m_high << " " << std::endl;
  for (auto x : m_entries) {
    file << x << " ";
  }
  file << std::endl;
  file.close();
}

void
Hist::save(std::ofstream& filehandle) const
{
  filehandle << m_steps << " " << m_low << " " << m_high << " " << std::endl;
  for (auto x : m_entries) {
    filehandle << x << " ";
  }
  filehandle << std::endl;
}

bool
Hist::is_running()
{
  return true;
}

void
Hist::run(dunedaq::dataformats::TriggerRecord& tr)
{
  dunedaq::dqm::Decoder dec;
  auto wibframes = dec.decode(tr);

  for (auto fr : wibframes) {
    for (int ich = 0; ich < 256; ++ich) {
      this->fill(fr->get_channel(ich));
    }
  }
  this->save("Hist/hist.txt");
}

void
Hist::clean()
{
  m_sum = 0;
  m_nentries = 0;
  for (auto& elem : m_entries) {
    elem = 0;
  }
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_HIST_HPP_
