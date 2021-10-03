/**
 * @file ChannelMapFiller.hpp Implementation of an algorithm to fill the channel map
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_CHANNELMAPFILLER_HPP_
#define DQM_SRC_CHANNELMAPFILLER_HPP_

// DQM
#include "AnalysisModule.hpp"
#include "ChannelMap.hpp"

#include "dataformats/TriggerRecord.hpp"

#include <string>

namespace dunedaq::dqm {

class ChannelMapFiller : public AnalysisModule{
  std::atomic<bool> m_is_running;
  std::string m_name;

public:
  ChannelMapFiller(std::string name);
  void run(dunedaq::dataformats::TriggerRecord& tr, ChannelMap& map, RunningMode mode, std::string kafka_address);
  bool is_running();

};

void
ChannelMapFiller::run(dunedaq::dataformats::TriggerRecord& tr, ChannelMap& map, RunningMode mode, std::string kafka_address)
{
  m_is_running.store(true);
  map.fill(tr);
  m_is_running.store(false);
}

bool
ChannelMapFiller::is_running()
{
  return m_is_running;
}

ChannelMapFiller::ChannelMapFiller(std::string name)
  : m_name(name) {
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_CHANNELMAPFILLER_HPP_
