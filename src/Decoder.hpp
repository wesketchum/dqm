#pragma once

/**
 * @file Decoder.hpp Implementation of decoders used in DQM analysis modules
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "dataformats/wib/WIBFrame.hpp"
#include "dataformats/Fragment.hpp"
#include "dataformats/TriggerRecord.hpp"

#include <vector>

namespace dunedaq{
namespace DQM{

class Decoder {
public:
  std::vector<dataformats::WIBFrame> decode(dunedaq::dataformats::TriggerRecord &record);

};

std::vector<dataformats::WIBFrame> decodewib(dataformats::TriggerRecord &record){
  std::vector<std::unique_ptr<dataformats::Fragment>>& fragments = record.get_fragments_ref();
  // std::cout << "Size of fragments is " << fragments.size() << std::endl;
  // for (auto &p: fragments) {
  //   TLOG() << "Fragment: " << p->get_trigger_number() << " " << p->get_size();
  //   auto datap = p->get_size();
  // }
  // std::ofstream file("Hist/hist.txt");

  // rolling_index++;
  // if(rolling_index >= 20)
  //   rolling_index -= 10;
  // TLOG() << "rolling_indx" << rolling_index;


  // std::vector<Hist> v;
  // for(int i=0; i<256; ++i)
  //     v.push_back(Hist(100, 0, 5000));

  auto &fr = fragments[0];
  auto data = fr->get_data();

  std::vector<std::pair<void*, size_t>> frag_pieces;
  size_t offset = sizeof(dataformats::FragmentHeader);
  void *data_array = malloc(5568);

  std::vector<dataformats::WIBFrame> vec;

  for(int iframe=0; iframe<100; ++iframe){
    if(offset + 5568 >= fr->get_size()){
      TLOG() << "EXITING!!!!!!!!!!!!" << iframe << " " << fr->get_size();
      break;
    }
    memcpy(data_array, (char*)data + offset, 5568);
    auto pair = std::make_pair<void*, size_t> (static_cast<void*>(data_array), (size_t)5568);
    frag_pieces.push_back(pair);
    offset += 5568;
    dataformats::WIBFrame *frame = static_cast<dataformats::WIBFrame*> (frag_pieces[iframe].first);
    vec.push_back(*frame);

    // for(int i=0; i<256; ++i) {
    //     // TLOG() << "Channel i: " << frame->get_channel(i);
    //     // TLOG() << "Channel i: " << frame->get_channel(i);
    //     v[i].fill(frame->get_channel(i));
    // }
    // if (iframe == rolling_index) {
    //   for(int i=0; i<256; ++i)
    //     v[i].save(file);
    //   file.close();
    //   return;
    // }
  } // frame loop
  return vec;

}

std::vector<dataformats::WIBFrame> Decoder::decode(dataformats::TriggerRecord &record){
  return decodewib(record);
}
  
} // namespace dunedaq
} // namespace DQM
