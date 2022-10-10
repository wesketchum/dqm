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
#include "dqm/AnalysisModule.hpp"
#include "ChannelMap.hpp"
#include "dqm/DQMFormats.hpp"

#include "daqdataformats/TriggerRecord.hpp"
#include "detdataformats/wib/WIBFrame.hpp"
#include "detdataformats/wib2/WIB2Frame.hpp"

#include <memory>
#include <string>

namespace dunedaq::dqm {

class ChannelMapFiller : public AnalysisModule
{
  std::string m_name;
  std::string m_cmap_name;

public:
  ChannelMapFiller(std::string name, std::string cmap_name);

  std::unique_ptr<daqdataformats::TriggerRecord>
  run(std::unique_ptr<daqdataformats::TriggerRecord> record,
      DQMArgs& args, DQMInfo& info);
};

std::unique_ptr<daqdataformats::TriggerRecord>
ChannelMapFiller::run(std::unique_ptr<daqdataformats::TriggerRecord> record,
                      DQMArgs& args, DQMInfo&)
{
  set_is_running(true);

  // Prevent running multiple times
  if (args.map->is_filled()) {
    return record;
  }

  std::map<std::string, std::string> map_names {
  {"HD", "ProtoDUNESP1ChannelMap"},
  {"VD", "VDColdboxChannelMap"},
  {"PD2HD", "PD2HDChannelMap"},
  {"HDCB", "HDColdboxChannelMap"}
  };

  args.map.reset(new ChannelMap(map_names[m_cmap_name]));

  if (args.frontend_type == "wib") {
    args.map->fill<detdataformats::wib::WIBFrame>(*record);
  }
  else if (args.frontend_type == "wib2") {
    args.map->fill<detdataformats::wib2::WIB2Frame>(*record);
  }
  set_is_running(false);
  return record;
}

ChannelMapFiller::ChannelMapFiller(std::string name, std::string cmap_name)
  : m_name(name)
{
  if (cmap_name != "HD" && cmap_name != "VD" && cmap_name != "PD2HD" && cmap_name != "HDCB") {
    TLOG() << "Wrong channel map name";
  } else {
    m_cmap_name = cmap_name;
  }
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_CHANNELMAPFILLER_HPP_
