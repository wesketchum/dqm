#pragma once

/**
 * @file HistContainer.hpp Implementation of a container of Hist objects
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

// dqm
#include "AnalysisModule.hpp"
#include "Decoder.hpp"
#include "Hist.hpp"
#include "Exporter.hpp"
#include "dqm/Types.hpp"

#include "dataformats/TriggerRecord.hpp"


namespace dunedaq::dqm{

class HistContainer : public AnalysisModule{
  std::string m_name;
  std::vector<Hist> histvec;
  bool m_run_mark;
  int m_size;

public:

  HistContainer(std::string name, int nhist, int steps, double low, double high);

  void run(dunedaq::dataformats::TriggerRecord &tr, RunningMode mode=RunningMode::kNormal);
  void transmit(const std::string &topicname, int run_num, time_t timestamp) const;
  bool is_running();
};

  HistContainer::HistContainer(std::string name, int nhist, int steps, double low, double high)
    : m_name(name), m_run_mark(false), m_size(nhist)
{
  for(int i=0; i<m_size; ++i)
    histvec.emplace_back(Hist(steps, low, high));
}

  void HistContainer::run(dunedaq::dataformats::TriggerRecord &tr, RunningMode mode){
  m_run_mark = true;
  dunedaq::dqm::Decoder dec;
  auto wibframes = dec.decode(tr);

  for(auto &fr:wibframes){
    for(int ich=0; ich<m_size; ++ich)
    histvec[ich].fill(fr.get_channel(ich));
  }

  //Transmit via kafka
  if (mode == RunningMode::kNormal){
    this->transmit("testdunedqm", tr.get_header_ref().get_run_number(), tr.get_header_ref().get_trigger_timestamp());
  }

  for(int ich=0; ich<m_size; ++ich)
    histvec[ich].save("Hist/" + m_name + "-" + std::to_string(ich) + ".txt");
  m_run_mark = false;
}

bool HistContainer::is_running(){
  return m_run_mark;
}

void HistContainer::transmit(const std::string &topicname, int run_num, time_t timestamp) const
{
  std::stringstream csv_output;
  std::string datasource = "TESTSOURCE";
  std::string dataname   = this->m_name;
  std::string axislabel = "TESTLABEL";
  std::stringstream metadata;
  metadata << histvec[0].m_steps << " " << histvec[0].m_low << " " << histvec[0].m_high;

  int subrun = 0;
  int event = 0;
  
  //Construct CSV output
  csv_output << datasource << ";" << dataname << ";" << run_num << ";" << subrun << ";" << event << ";" << timestamp << ";" << metadata.str() << ";";
  csv_output << axislabel << "\n";
  for (int ich = 0; ich < 256; ++ich)
  {
    csv_output << "Histogram_" << ich+1 << "\n";
    for (auto x: histvec[ich].m_entries) csv_output << x << " ";
    csv_output << "\n";
  }
  //csv_output << "\n"; 
  
  //Transmit
  KafkaExport(csv_output.str(), topicname);
}


}
