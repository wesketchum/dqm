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

  // Three dimensional array of 4x2x256 holding the offline channel corresponding to each combination of 
  // slot, fiber and frame channel
  vvvi vec;
  vvvi planevec; 

public:
  ChannelMapVD();
  void fill(dataformats::TriggerRecord &tr);
  std::map<int, std::map<int, std::pair<int, int>>> get_map();
};

ChannelMapVD::ChannelMapVD()
{
  vec = vvvi(4, vvi(2, vi(256, -1)));
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
    std::string path = std::string(std::getenv("DQM_SHARE"));
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

      getline(ss, str, ',');
      connector_pin = std::stoi(str);

      getline(ss, str, ',');
      ce_board = std::stoi(str);

      getline(ss, str, ',');
      ceb_channel = std::stoi(str);

      getline(ss, str, ',');
      asic = std::stoi(str);

      getline(ss, str, ',');
      asic_channel = std::stoi(str);

      int wib = (ce_board - 1) % 4;
      int link = ((ce_board - 1) / 2) % 2;
      int channel = ceb_channel;

      vec[wib][link][channel] = std::stoi(strip_number.substr(1));

      if (strip_number[0] == 'U') {
        planevec[wib][link][channel] = 0;
      }
      else if (strip_number[0] == 'Y') {
        planevec[wib][link][channel] = 1;
      }
      else if (strip_number[0] == 'Z') {
        planevec[wib][link][channel] = 2;
      }

      TLOG() << "Setting wib " << wib << " link " << link << " channel " << channel << " to offline channel " << vec[wib][link][channel] << " and plane " << planevec[wib][link][channel];
    }

  }

  if (is_filled()) {
    TLOG() << "ChannelMapVD already filled";
    return;
  }

  Decoder dec;
  auto wibframes = dec.decode(tr);

  TLOG() << "Got " << wibframes.size() << " frames";

  // If we get no frames then return and since
  // the map is not filled it will run again soon
  if (wibframes.size() == 0)
    return;

  std::set<std::tuple<int, int, int>> frame_numbers;
  for (auto& [key, value] : wibframes) {
    // This is one link so we push back one element to m_map
    for (auto& fr : value) {
      TLOG() << "New frame";
      int crate = fr->get_wib_header()->crate_no;
      int slot = fr->get_wib_header()->slot_no;
      int fiber = fr->get_wib_header()->fiber_no;
      TLOG() << crate << " " << slot << " " << fiber;
      auto tmp = std::make_tuple<int, int, int>((int)crate, (int)slot, (int)fiber);
      if (frame_numbers.find(tmp) == frame_numbers.end()) {
        frame_numbers.insert(tmp);
      }
      else {
        continue;
      }
      for (int ich=0; ich < CHANNELS_PER_LINK; ++ich) {
        // fiber takes the values 1 and 2 but it was indexed in vec and planevec
        // beggining with 0 so we need to subtract 1
        auto channel = vec[slot][fiber-1][ich];
        auto plane = planevec[slot][fiber-1][ich];
        m_map[plane][channel] = {key, ich};
      }
    }
  }
  TLOG() << "Channel mapping done, size of the map is " << m_map[0].size() << " " << m_map[1].size() << " " << m_map[2].size();

  TLOG() << "Setting m_is_filled to true";
  m_is_filled = true;
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_CHANNELMAPVD_HPP_
