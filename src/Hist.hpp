/**
 * @file Hist.hpp Simple 1D histogram implementation
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include <vector>
#include <string>
#include <ostream>
#include <fstream>
#include <iostream>

/**
 * Basic 1D histogram that counts entries
 * It only supports uniform binning
 * Overflow and underflow is not supported yet
 */
class Hist {

  int find_bin(double x) const;

public:
  double m_low, m_high, m_step_size;
  int m_nentries;
  double m_sum;
  
  int m_steps;
  std::vector<int> m_entries;

  Hist(int steps, double low, double high);
  int fill(double x);

  void save(const std::string &filename) const;
  void save(std::ofstream &filehandle) const;
};



Hist::Hist(int steps, double low, double high)
  : m_low(low), m_high(high), m_steps(steps)
{
  m_entries = std::vector<int> (steps, 0);
  m_step_size = (high - low) / steps;
}

int Hist::find_bin(double x) const {
    return (x - m_low) / m_step_size;
}
  
int Hist::fill(double x){
  int bin = find_bin(x);
  // Underflow, do nothing
  if(bin < 0) return -1;

  // Overflow, do nothing
  if(bin >= m_steps) return -1;

  m_entries[bin]++;
  m_nentries++;
  m_sum += x;
  return bin;
}

void Hist::save(const std::string &filename) const {
  std::ofstream file;
  file.open(filename);
  file << m_steps << " " << m_low << " " << m_high << " " << std::endl;
  for (auto x: m_entries)
    file << x << " ";
  file << std::endl;
}

void Hist::save(std::ofstream &filehandle) const {
  filehandle << m_steps << " " << m_low << " " << m_high << " " << std::endl;
  for (auto x: m_entries)
    filehandle << x << " ";
  filehandle << std::endl;
}
