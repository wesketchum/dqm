/**
 * @file ChannelMap.hpp Implementation of a base class for channel maps
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_CHANNELMAP_HPP_
#define DQM_SRC_CHANNELMAP_HPP_

// DQM
#include "dataformats/TriggerRecord.hpp"

namespace dunedaq::dqm {

class ChannelMap {

public:
  bool m_is_filled = false;

  ChannelMap();
  virtual ~ChannelMap();
  int get_channel(int channel);
  int get_plane(int channel);
  virtual bool is_filled();
  virtual void fill(dataformats::TriggerRecord &tr);
  virtual std::map<int, std::map<int, std::pair<int, int>>> get_map();
};


ChannelMap::ChannelMap()
{
}

ChannelMap::~ChannelMap() {
}


std::map<int, std::map<int, std::pair<int, int>>>
ChannelMap::get_map(){
  //throw issue
  std::map<int, std::map<int, std::pair<int, int>>> ret;
  return ret;
}

void
ChannelMap::fill(dataformats::TriggerRecord&){
  //throw issue
}

bool
ChannelMap::is_filled() {
  return m_is_filled;
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_CHANNELMAP_HPP_
