/**
 * @file ChannelMapHD.hpp Implementation of the channel map for the horizontal drift
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_CHANNELMAPHD_HPP_
#define DQM_SRC_CHANNELMAPHD_HPP_

// DQM
#include "ChannelMap.hpp"
#include "Decoder.hpp"
#include "Constants.hpp"

#include "dataformats/TriggerRecord.hpp"
#include "readout/chmap/PdspChannelMapService.hpp"

#include <stdlib.h>
#include <set>

namespace dunedaq::dqm {

unsigned int getOfflineChannel(swtpg::PdspChannelMapService& channelMap, // NOLINT(build/unsigned)
                               const dunedaq::dataformats::WIBFrame* frame,
                               unsigned int ch);

class ChannelMapHD : public ChannelMap {
  std::map<int, std::map<int, std::pair<int, int>>> m_map;

public:
  ChannelMapHD();
  int get_channel(int channel);
  int get_plane(int channel);
  bool is_filled();
  void fill(dataformats::TriggerRecord &tr);
  std::map<int, std::map<int, std::pair<int, int>>> get_map();
};

ChannelMapHD::ChannelMapHD()
{
}

std::map<int, std::map<int, std::pair<int, int>>>
ChannelMapHD::get_map(){
  return m_map;
}

void
ChannelMapHD::fill(dataformats::TriggerRecord &tr){

  if (is_filled()) {
    TLOG(5) << "ChannelMapHD already filled";
    return;
  }

  dunedaq::dqm::Decoder dec;
  auto wibframes = dec.decode(tr);

  std::unique_ptr<swtpg::PdspChannelMapService> channelmap;
  // There is one env variable $PACKAGE_SHARE for each
  // DUNEDAQ package
  std::string path = std::string(getenv("READOUT_SHARE"));
  std::string channel_map_rce = std::string(path) + "/config/protoDUNETPCChannelMap_RCE_v4.txt";
  std::string channel_map_felix = std::string(path) + "/config/protoDUNETPCChannelMap_FELIX_v4.txt";
  channelmap.reset(new swtpg::PdspChannelMapService(channel_map_rce, channel_map_felix));


  // If we get no frames then return and since
  // the map is not filled it will run again soon
  if (wibframes.size() == 0)
    return;

  std::set<std::tuple<int, int, int>> frame_numbers;
  for (auto& [key, value] : wibframes) {
    // This is one link so we push back one element to m_map
    for (auto& fr : value) {
      int crate = fr->get_wib_header()->crate_no;
      int slot = fr->get_wib_header()->slot_no;
      int fiber = fr->get_wib_header()->fiber_no;
      auto tmp = std::make_tuple<int, int, int>((int)crate, (int)slot, (int)fiber);
      if (frame_numbers.find(tmp) == frame_numbers.end()) {
        frame_numbers.insert(tmp);
      }
      else {
        continue;
      }
      for (int ich=0; ich < CHANNELS_PER_LINK; ++ich) {
        auto channel = getOfflineChannel(*channelmap, fr, ich);
        auto plane = channelmap->PlaneFromOfflineChannel(channel);
        m_map[plane][channel] = {key, ich};
      }
    }
  }
  TLOG(10) << "Channel mapping done, size of the map is " << m_map[0].size() << " " << m_map[1].size() << " " << m_map[2].size();

  TLOG(5) << "Channel Map for the HD created";
  m_is_filled = true;

}

bool
ChannelMapHD::is_filled() {
  return m_is_filled;
}

unsigned int getOfflineChannel(swtpg::PdspChannelMapService& channelMap, // NOLINT(build/unsigned)
                               const dunedaq::dataformats::WIBFrame* frame,
                               unsigned int ch) // NOLINT(build/unsigned)
{
  // handle 256 channels on two fibers -- use the channel
  // map that assumes 128 chans per fiber (=FEMB) (Copied
  // from PDSPTPCRawDecoder_module.cc)
  int crate = frame->get_wib_header()->crate_no;
  int slot = frame->get_wib_header()->slot_no;
  int fiber = frame->get_wib_header()->fiber_no;

  unsigned int fiberloc = 0; // NOLINT(build/unsigned)
  if (fiber == 1) {
    fiberloc = 1;
  } else if (fiber == 2) {
    fiberloc = 3;
  } else {
    TLOG() << " Fiber number " << fiber << " is expected to be 1 or 2 -- revisit logic";
    fiberloc = 1;
  }

  unsigned int chloc = ch; // NOLINT(build/unsigned)
  if (chloc > 127) {
    chloc -= 128;
    fiberloc++;
  }

  unsigned int crateloc = crate; // NOLINT(build/unsigned)
  unsigned int offline =         // NOLINT(build/unsigned)
    channelMap.GetOfflineNumberFromDetectorElements(
      crateloc, slot, fiberloc, chloc, swtpg::PdspChannelMapService::kFELIX);
  return offline;
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_CHANNELMAPHD_HPP_
