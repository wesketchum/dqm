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
  void transmit(const std::string &topicname, int run_num, time_t timestamp);
  void clean();
  void save_and_clean(uint64_t timestamp);
  bool is_running();

  int frames_run = 0;
  int max_frames = 1;
  int filename_index = 0;
  int current_index = 0;
};

HistContainer::HistContainer(std::string name, int nhist, int steps, double low, double high)
    : m_name(name), m_run_mark(false), m_size(nhist)
{
  for(int i=0; i<m_size; ++i)
    histvec.emplace_back(Hist(steps, low, high));
}

void
HistContainer::run(dunedaq::dataformats::TriggerRecord &tr, RunningMode mode)
{
  m_run_mark = true;
  dunedaq::dqm::Decoder dec;
  auto wibframes = dec.decode(tr);
  std::uint64_t timestamp = 0;

  for(auto fr:wibframes){

    for(int ich=0; ich<m_size; ++ich)
    histvec[ich].fill(fr->get_channel(ich));

    // Debug mode - save to a file
    // if (mode != RunningMode::kLocalProcessing) continue;
    if(++frames_run == max_frames)
    {
        timestamp = fr->get_wib_header()->get_timestamp();
        if (mode == RunningMode::kLocalProcessing)
          save_and_clean(timestamp);
        else if (mode == RunningMode::kNormal)
          transmit("testdunedqm", tr.get_header_ref().get_run_number(), tr.get_header_ref().get_trigger_timestamp());
        frames_run = 0;
        ++current_index;
        if(current_index == 2000)
        {
          ++filename_index;
          current_index = 0;
        }
    }
  }

  //Transmit via kafka
  // if (mode == RunningMode::kNormal){
  //
  // }

  m_run_mark = false;

  // clean();
}

bool
HistContainer::is_running()
{
  return m_run_mark;
}

void
HistContainer::transmit(const std::string &topicname, int run_num, time_t timestamp)
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

  clean();
}

void
HistContainer::clean()
{
  for (int ich=0; ich<m_size; ++ich)
    histvec[ich].clean();
}

void
HistContainer::save_and_clean(uint64_t timestamp)
{
  // TLOG() << "Saving and cleaning histograms";

  std::ofstream file;
  file.open("Hist/" + m_name + "-" + std::to_string(filename_index) + ".txt", std::ios_base::app);
  file << timestamp << std::endl;

  for(int ich=0; ich<m_size; ++ich)
    file << histvec[ich].m_sum << " ";
  file << std::endl;
  file.close();

  clean();
}

} //end of namespace dunedaq::dqm
