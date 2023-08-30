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
#include "dqm/ChannelMap.hpp"
#include "dqm/DQMFormats.hpp"

#include "daqdataformats/TriggerRecord.hpp"
#include "fddetdataformats/WIBFrame.hpp"
#include "fddetdataformats/WIB2Frame.hpp"
#include "fddetdataformats/WIBEthFrame.hpp"
#include "fddetdataformats/DAPHNEFrame.hpp"
#include "fddetdataformats/DAPHNEStreamFrame.hpp"

#include <memory>
#include <string>

namespace dunedaq::dqm {

class ChannelMapFiller : public AnalysisModule
{
  std::string m_name;
  std::string m_cmap_name;

public:
  ChannelMapFiller(std::string name, std::string cmap_name);

  void
  run(std::shared_ptr<daqdataformats::TriggerRecord> record,
      DQMArgs& args, DQMInfo& info) override;
};

void
ChannelMapFiller::run(std::shared_ptr<daqdataformats::TriggerRecord> record,
                      DQMArgs& args, DQMInfo& /*info*/)
{
  set_is_running(true);

  // Prevent running multiple times
  if (args.map->is_filled()) {
    return;
  }

  std::map<std::string, std::string> map_names {
  {"HD", "ProtoDUNESP1ChannelMap"},
  {"VD", "VDColdboxChannelMap"},
  {"PD2HD", "PD2HDChannelMap"},
  {"HDCB", "HDColdboxChannelMap"}
  };

  args.map.reset(new ChannelMap(map_names[m_cmap_name]));

  if (args.frontend_type == "wib") {
    args.map->fill<fddetdataformats::WIBFrame>(record);
  }
  else if (args.frontend_type == "wib2") {
    args.map->fill<fddetdataformats::WIB2Frame>(record);
  }
  else if (args.frontend_type == "wibeth") {
    args.map->fill<fddetdataformats::WIBEthFrame>(record);
  }
  else if (args.frontend_type == "daphne") {
    args.map->fill<fddetdataformats::DAPHNEFrame>(record);
  }
  else if (args.frontend_type == "daphnestream") {
    args.map->fill<fddetdataformats::DAPHNEStreamFrame>(record);
  }
  set_is_running(false);
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
