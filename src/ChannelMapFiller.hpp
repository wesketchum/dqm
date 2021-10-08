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
  std::string m_name;

public:
  ChannelMapFiller(std::string name);
  void run(dunedaq::dataformats::TriggerRecord& tr, ChannelMap& map, std::string kafka_address);

};

void
ChannelMapFiller::run(dunedaq::dataformats::TriggerRecord& tr, ChannelMap& map, std::string)
{
  m_run_mark.store(true);
  map.fill(tr);
  m_run_mark.store(false);
}

ChannelMapFiller::ChannelMapFiller(std::string name)
  : m_name(name) {
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_CHANNELMAPFILLER_HPP_
