#pragma once

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

// DQM
#include "AnalysisModule.hpp"
#include "Decoder.hpp"

#include "dataformats/TriggerRecord.hpp"

/**
 * Basic 1D histogram that counts entries
 * It only supports uniform binning
 * Overflow and underflow is not supported yet
 */
namespace dunedaq::DQM{

class Hist : public AnalysisModule{

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

  bool is_running();
  void run(dunedaq::dataformats::TriggerRecord &tr);
};



Hist::Hist(int steps, double low, double high)
  : m_low(low), m_high(high), m_steps(steps)
{
  m_entries = std::vector<int> (steps, 0);
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
  if(bin < 0) return -1;

  // Overflow, do nothing
  if(bin >= m_steps) return -1;

  m_entries[bin]++;
  m_nentries++;
  m_sum += x;
  return bin;
}

void
Hist::save(const std::string &filename) const
{
  std::ofstream file;
  file.open(filename);
  file << m_steps << " " << m_low << " " << m_high << " " << std::endl;
  for (auto x: m_entries)
    file << x << " ";
  file << std::endl;
}

void
Hist::save(std::ofstream &filehandle) const
{
  filehandle << m_steps << " " << m_low << " " << m_high << " " << std::endl;
  for (auto x: m_entries)
    filehandle << x << " ";
  filehandle << std::endl;
}

bool
Hist::is_running()
{
  return true;
}

void
Hist::run(dunedaq::dataformats::TriggerRecord &tr)
{
  dunedaq::DQM::Decoder dec;
  auto wibframes = dec.decode(tr);

  for(auto &fr:wibframes){
    for(int ich=0; ich<256; ++ich)
      this->fill(fr.get_channel(ich));
  }
  this->save("Hist/hist.txt");
  
}

class HistLink : public AnalysisModule{
  std::string m_name;
  std::vector<Hist> histvec;
  bool m_run_mark;

public:

  HistLink(std::string name, int steps, double low, double high);

  void run(dunedaq::dataformats::TriggerRecord &tr);
  bool is_running();
};

HistLink::HistLink(std::string name, int steps, double low, double high)
  : m_name(name), m_run_mark(false)
{
  for(int i=0; i<256; ++i){
    Hist hist(steps, low, high);
    histvec.push_back(hist);
  }
}

void HistLink::run(dunedaq::dataformats::TriggerRecord &tr){
  m_run_mark = true;
  dunedaq::DQM::Decoder dec;
  auto wibframes = dec.decode(tr);

  for(auto &fr:wibframes){
    for(int ich=0; ich<256; ++ich)
    histvec[ich].fill(fr.get_channel(ich));
  }

  for(int ich=0; ich<256; ++ich)
    histvec[ich].save("Hist/" + m_name + "-" + std::to_string(ich) + ".txt");
  m_run_mark = false;
}

bool HistLink::is_running(){
  return m_run_mark;
}
 

} // namespace dunedaq::DQM
