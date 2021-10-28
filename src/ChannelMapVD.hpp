/**
 * @file ChannelMapVD.hpp Implementation of the channel map for the vertical drift
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_CHANNELMAPVD_HPP_
#define DQM_SRC_CHANNELMAPVD_HPP_

// DQM
#include "Decoder.hpp"
#include "Constants.hpp"

#include "dataformats/TriggerRecord.hpp"

#include <stdlib.h>
#include <set>

namespace dunedaq::dqm {

typedef std::vector<int> vi;
typedef std::vector<vi> vvi;
typedef std::vector<vvi> vvvi;

class ChannelMapVD : public ChannelMap{

  std::map<int, std::map<int, std::pair<int, int>>> m_map;

  // Three dimensional array of 4x2x256 (4 WIBS x 2 LINKS x 256 channels)
  // holding the offline channel corresponding to each combination of slot,
  // fiber and frame channel
  vvvi channelvec;
  vvvi planevec; 

public:
  ChannelMapVD();
  void fill(dataformats::TriggerRecord &tr);
  std::map<int, std::map<int, std::pair<int, int>>> get_map();
};

ChannelMapVD::ChannelMapVD()
{
  channelvec = vvvi(4, vvi(2, vi(256, -1)));
  planevec = vvvi(4, vvi(2, vi(256, -1)));
}

std::map<int, std::map<int, std::pair<int, int>>>
ChannelMapVD::get_map(){
  return m_map;
}

void
ChannelMapVD::fill(dataformats::TriggerRecord &tr){

  // Read each csv file
  // They have the following format
  // Strip Number,Connector #,Connector Pin #,CE Board #,CEB Channel #,ASIC,ASIC Channel
  std::string strip_number;
  int connector, connector_pin, ce_board, ceb_channel, asic, asic_channel;
  for (int i = 1; i <= 8; ++i) {
    std::ifstream csv;
    // There is one env variable $PACKAGE_SHARE for each
    // DUNEDAQ package
    auto env = std::getenv("DQM_SHARE");
    // Make sure the env variable can be retrieved
    if (env == nullptr) {
      throw InvalidEnvVariable("READOUT_SHARE");
      return;
    }

    std::string path = std::string(env);
    csv.open(path + "/config/" + "AB" + std::to_string(i) + ".csv");
    bool first_line = true;
    std::string line;
    while (getline(csv, line)) {
      // The first line is a header line so we skip it
      if (first_line) {
        first_line = false;
        continue;
      }
      std::stringstream ss(line);
      std::string str;

      getline(ss, strip_number, ',');

      getline(ss, str, ',');
      connector = std::stoi(str);
      (void)connector; // Silences unused warning, does nothing

      getline(ss, str, ',');
      connector_pin = std::stoi(str);
      (void)connector_pin; // Silences unused warning, does nothing

      getline(ss, str, ',');
      ce_board = std::stoi(str);

      getline(ss, str, ',');
      ceb_channel = std::stoi(str);

      getline(ss, str, ',');
      asic = std::stoi(str);
      (void)asic; // Silences unused warning, does nothing

      getline(ss, str, ',');
      asic_channel = std::stoi(str);
      (void)asic_channel; // Silences unused warning, does nothing

      // Each CE board is connected to one connector of one WIB and each WIB has
      // 4 connectors. CE board 1 maps to the connector 1 in WIB 1, CE board 2
      // to the connector 2 of WIB 1 and so on. Numbering begins at 1 for CE
      // boards but at zero for wibs
      int wib = (ce_board - 1) % 4;

      // Each WIB has two links so assuming that the first two connectors are
      // connected to the first link and the last two connectors are connected
      // to the second link we have (with link being either 0 or 1)
      int link = ((ce_board - 1) / 2) % 2;

      // CEB channel goes between 0 and 127 for each board so 0 - 127 will
      // correspond to the first connector and 128 - 255 to the second connector
      // If the ce_board is odd then it's the first connector and if it's even
      // then it's the second connector
      int channel = ceb_channel + 128 * ((ce_board - 1) % 2);

      channelvec[wib][link][channel] = std::stoi(strip_number.substr(1));

      if (strip_number[0] == 'U') {
        planevec[wib][link][channel] = 0;
      }
      else if (strip_number[0] == 'Y') {
        planevec[wib][link][channel] = 1;
      }
      else if (strip_number[0] == 'Z') {
        planevec[wib][link][channel] = 2;
      }

    }

  }

  if (is_filled()) {
    TLOG_DEBUG(5) << "ChannelMapVD already filled";
    return;
  }

  Decoder dec;
  auto wibframes = dec.decode(tr);

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
        // fiber takes the values 1 and 2 but it was indexed in channelvec and planevec
        // beggining with 0 so we need to subtract 1
        auto channel = channelvec[slot][fiber-1][ich];
        auto plane = planevec[slot][fiber-1][ich];
        m_map[plane][channel] = {key, ich};
      }
    }
  }

  TLOG_DEBUG(5) << "Channel Map for the VD created";
  m_is_filled = true;
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_CHANNELMAPVD_HPP_
