/**
 * @file ChannelMap.hpp Implementation of a base class for channel maps
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_CHANNELMAP_HPP_
#define DQM_SRC_CHANNELMAP_HPP_

#include "detchannelmaps/TPCChannelMap.hpp"
#include "daqdataformats/TriggerRecord.hpp"
#include "logging/Logging.hpp"

#include "Decoder.hpp"
#include "dqm/FormatUtils.hpp"

#include <map>
#include <memory>
#include <utility>

namespace dunedaq::dqm {

class ChannelMap
{

  std::map<int, std::map<int, std::pair<int, int>>> m_map;

public:
  std::shared_ptr<dunedaq::detchannelmaps::TPCChannelMap> m_chmap_service;
  bool m_is_filled = false;

  ChannelMap();
  ChannelMap(std::string& name);
  int get_channel(int channel);
  int get_plane(int channel);
  bool is_filled();

  template <class T>
  void fill(daqdataformats::TriggerRecord& tr);

  std::map<int, std::map<int, std::pair<int, int>>> get_map();
};


ChannelMap::ChannelMap() {
}

ChannelMap::ChannelMap(std::string& name) {
  m_chmap_service = dunedaq::detchannelmaps::make_map(name);
}

std::map<int, std::map<int, std::pair<int, int>>>
ChannelMap::get_map()
{
  return m_map;
}

template <class T>
void
ChannelMap::fill(daqdataformats::TriggerRecord& record)
{
  if (is_filled()) {
    TLOG_DEBUG(5) << "ChannelMapHD already filled";
    return;
  }

  auto frames = decode<T>(record, 0);

  // If we get no frames then return and since
  // the map is not filled it will run again soon
  if (frames.size() == 0)
    return;

  std::set<std::tuple<int, int, int>> frame_numbers;
  for (auto& [key, value] : frames) {
    // This is one link so we push back one element to m_map
    for (auto& fr : value) {
      int crate = get_crate<T>(fr);
      int slot = get_slot<T>(fr);
      int fiber = get_fiber<T>(fr);
      auto tmp = std::make_tuple(crate, slot, fiber);
      if (frame_numbers.find(tmp) == frame_numbers.end()) {
        frame_numbers.insert(tmp);
      } else {
        continue;
      }
      for (int ich = 0; ich < 256; ++ich) {
        auto channel = m_chmap_service->get_offline_channel_from_crate_slot_fiber_chan(crate, slot, fiber, ich);
        auto plane = m_chmap_service->get_plane_from_offline_channel(channel);
        m_map[plane][channel] = { key, ich };
      }
    }
  }
  TLOG_DEBUG(10) << "Channel mapping done, size of the map is " << m_map[0].size() << " " << m_map[1].size() << " "
                 << m_map[2].size();

  TLOG_DEBUG(5) << "Channel Map for the HD created";
  m_is_filled = true;
}

bool
ChannelMap::is_filled()
{
  return m_is_filled;
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_CHANNELMAP_HPP_
