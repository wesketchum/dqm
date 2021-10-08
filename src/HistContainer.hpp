/**
 * @file HistContainer.hpp Implementation of a container of Hist objects
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_HISTCONTAINER_HPP_
#define DQM_SRC_HISTCONTAINER_HPP_

// DQM
#include "AnalysisModule.hpp"
#include "Decoder.hpp"
#include "Exporter.hpp"
#include "ChannelMap.hpp"
#include "dqm/Hist.hpp"
#include "dqm/Types.hpp"

#include "dataformats/TriggerRecord.hpp"

#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <cstdlib>

namespace dunedaq::dqm {


class HistContainer : public AnalysisModule
{
  std::string m_name;
  std::vector<Hist> histvec;
  int m_size;
  std::map<int, std::string> m_to_send;
  bool m_only_mean_rms = false;

public:
  HistContainer(std::string name, int nhist, int steps, double low, double high, bool only_mean=false);

  void run(dunedaq::dataformats::TriggerRecord& tr, ChannelMap& map, RunningMode mode = RunningMode::kNormal, std::string kafka_address="");
  void transmit(std::string &kafka_address, ChannelMap& map, const std::string& topicname, int run_num, time_t timestamp);
  void transmit_mean_and_rms(std::string &kafka_address, ChannelMap& map, const std::string& topicname, int run_num, time_t timestamp);
  void clean();
  void append_to_string(std::uint64_t timestamp, ChannelMap& map);

};

HistContainer::HistContainer(std::string name, int nhist, int steps, double low, double high, bool only_mean)
  : m_name(name)
  , m_size(nhist)
  , m_only_mean_rms(only_mean)
{
  for (int i = 0; i < m_size; ++i)
    histvec.emplace_back(Hist(steps, low, high));
}

void
HistContainer::run(dunedaq::dataformats::TriggerRecord& tr, ChannelMap& map, RunningMode mode, std::string kafka_address)
{
  m_run_mark.store(true);
  dunedaq::dqm::Decoder dec;
  auto wibframes = dec.decode(tr);

  std::uint64_t timestamp = 0; // NOLINT(build/unsigned)

  // Main loop
  // If only the mean and rms are to be sent all frames are processed
  // and at the end the result is transmitted
  // If it's in the raw display mode then the result is saved for
  // every frame and sent at the end
  for (auto fr : wibframes) {
    auto timestamp = fr->get_wib_header()->get_timestamp();

    for (int ich = 0; ich < m_size; ++ich) {
      histvec[ich].fill(fr->get_channel(ich));
    }
    if (!m_only_mean_rms) {
      append_to_string(timestamp, map);
      clean();
    }
  }
  if (m_only_mean_rms) {
    transmit_mean_and_rms(kafka_address, map, "testdunedqm", tr.get_header_ref().get_run_number(), tr.get_header_ref().get_trigger_timestamp());
  }
  else {
    transmit(kafka_address, map, "testdunedqm", tr.get_header_ref().get_run_number(), tr.get_header_ref().get_trigger_timestamp());
  }
  clean();

  m_run_mark.store(false);

}

void
HistContainer::append_to_string(std::uint64_t timestamp, ChannelMap& map)
{
  auto channel_order = map.get_map();
  for (auto& [key, value] : channel_order) {
    m_to_send[key] += std::to_string(timestamp) + "\n";
    for (auto& [offch, ch] : value) {
      m_to_send[key] += std::to_string(static_cast<int>(histvec[ch].m_sum)) + " ";
    }
    m_to_send[key] += "\n";
  }
}

void
HistContainer::transmit(std::string& kafka_address, ChannelMap& map, const std::string& topicname, int run_num, time_t timestamp)
{
  // Placeholders
  std::string datasource = "TESTSOURCE";
  std::string dataname = m_name;
  std::string metadata = "";
  int subrun = 0;
  int event = 0;
  std::string partition = "PARTITION";
  std::string app_name = "APP_NAME";

  // One message is sent for every plane
  auto channel_order = map.get_map();

  for (auto& [key, value] : channel_order) {
    std::stringstream output;
    output << datasource << ";" << dataname << ";" << run_num << ";" << subrun
           << ";" << event << ";" << timestamp << ";" << metadata << ";"
           << partition << ";" << app_name << ";" << 0 << ";";
    output << "Plane " << key << "\n";
    output << m_to_send[key];
    TLOG() << "Size of the string: " << output.str().size();
    KafkaExport(kafka_address, output.str(), topicname);
  }
  m_to_send.clear();

}

void
HistContainer::transmit_mean_and_rms(std::string& kafka_address, ChannelMap& map, const std::string& topicname, int run_num, time_t timestamp)
{
  // Placeholders
  std::string datasource = "TESTSOURCE";
  std::string dataname = m_name;
  std::string metadata = "";
  int subrun = 0;
  int event = 0;
  std::string partition = "PARTITION";
  std::string app_name = "APP_NAME";

  // One message is sent for every plane
  auto channel_order = map.get_map();
  for (auto& [key, value] : channel_order) {
    std::stringstream output;
    output << datasource << ";" << dataname << ";" << run_num << ";" << subrun
           << ";" << event << ";" << timestamp << ";" << metadata << ";"
           << partition << ";" << app_name << ";" << 0 << ";";
    output << "Plane " << std::to_string(key) << "\n";
    output << "Mean\n";
    for (auto& [offch, ch] : value) {
      output << histvec[ch].mean() << " ";
    }
    output << "\n";
    output << "RMS\n";
    for (auto& [offch, ch] : value) {
      output << histvec[ch].std() << " ";
    }
    output << "\n";
    // TLOG() << output.str();
    KafkaExport(kafka_address, output.str(), topicname);
  }

}

void
HistContainer::clean()
{
  for (int ich = 0; ich < m_size; ++ich) {
    histvec[ich].clean();
  }
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_HISTCONTAINER_HPP_
