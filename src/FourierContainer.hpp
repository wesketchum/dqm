/**
 * @file FourierContainer.hpp Implementation of a container of Fourier objects
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_FOURIERCONTAINER_HPP_
#define DQM_SRC_FOURIERCONTAINER_HPP_

// DQM
#include "AnalysisModule.hpp"
#include "ChannelMap.hpp"
#include "Decoder.hpp"
#include "Exporter.hpp"
#include "dqm/Fourier.hpp"

#include "dataformats/TriggerRecord.hpp"

#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

namespace dunedaq::dqm {

class FourierContainer : public AnalysisModule
{
  std::string m_name;
  std::vector<Fourier> fouriervec;
  size_t m_size;
  int m_npoints;

public:
  FourierContainer(std::string name, int size, double inc, int npoints);

  void run(dunedaq::dataformats::TriggerRecord& tr, ChannelMap& map, std::string kafka_address="");
  void transmit(std::string &kafka_address, ChannelMap& map, const std::string& topicname, int run_num, time_t timestamp);
  void clean();

};

FourierContainer::FourierContainer(std::string name, int size, double inc, int npoints)
  : m_name(name),
    m_size(size),
    m_npoints(npoints)
{
  for (size_t i = 0; i < m_size; ++i)
    fouriervec.emplace_back(Fourier(inc, npoints));
}

void
FourierContainer::run(dunedaq::dataformats::TriggerRecord& tr, ChannelMap& map, std::string kafka_address)
{
  m_run_mark.store(true);
  dunedaq::dqm::Decoder dec;
  auto wibframes = dec.decode(tr);
  std::uint64_t timestamp = 0; // NOLINT(build/unsigned)

  for (auto& fr : wibframes) {

    for (size_t ich = 0; ich < m_size; ++ich)
      fouriervec[ich].fill(fr->get_channel(ich));
  }

  for (size_t ich = 0; ich < m_size; ++ich)
    fouriervec[ich].compute_fourier_normalized();

  transmit(kafka_address, map, "testdunedqm", tr.get_header_ref().get_run_number(), tr.get_header_ref().get_trigger_timestamp());

  m_run_mark.store(false);
}

void
FourierContainer::transmit(std::string &kafka_address, ChannelMap& map, const std::string& topicname, int run_num, time_t timestamp)
{
  // Placeholders
  std::string datasource = "TESTSOURCE";
  std::string dataname = m_name;
  std::string metadata = "";
  int subrun = 0;
  int event = 0;
  std::string partition = "PARTITION";
  std::string app_name = "APP_NAME";

  auto freq = fouriervec[0].get_frequencies();
  // One message is sent for every plane
  auto channel_order = map.get_map();
  for (auto& [key, value] : channel_order) {
    std::stringstream output;
    output << datasource << ";" << dataname << ";" << run_num << ";" << subrun
           << ";" << event << ";" << timestamp << ";" << metadata << ";"
           << partition << ";" << app_name << ";" << 0 << ";" << key << ";";
    for (auto& [offch, ch] : value) {
      output << offch << " ";
    }
    output << "\n";
    for (int i=0; i < freq.size(); ++i) {
      output << freq[i] << "\n";
      for (auto& [offch, ch] : value) {
        output << fouriervec[ch].get_transform(i) << " ";
      }
      output << "\n";
    }
    TLOG() << "Size of the string: " << output.str().size();
    // TLOG() << output.str();
    KafkaExport(kafka_address, output.str(), topicname);
  }

  clean();
}

void
FourierContainer::clean()
{
  for (int ich = 0; ich < m_size; ++ich) {
    fouriervec[ich].clean();
  }
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_FOURIERCONTAINER_HPP_
