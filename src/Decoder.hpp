/**
 * @file Decoder.hpp Implementation of decoders used in DQM analysis modules
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_DECODER_HPP_
#define DQM_SRC_DECODER_HPP_

#include "dataformats/Fragment.hpp"
#include "dataformats/TriggerRecord.hpp"
#include "dataformats/wib/WIBFrame.hpp"

#include <climits>
#include <memory>
#include <vector>

namespace dunedaq {
namespace dqm {

class Decoder
{
public:
  /**
   * @brief Decode a Trigger Record
   * @param record The Trigger Record to be decoded
   * @return A map whose keys are the GeoID indexes and the values are
   *         vectors of pointers to the wibframes
   */
  std::map<int, std::vector<dataformats::WIBFrame*>> decode(dunedaq::dataformats::TriggerRecord& record);
};

std::map<int, std::vector<dataformats::WIBFrame*>>
decodewib(dataformats::TriggerRecord& record)
{
  std::vector<std::unique_ptr<dataformats::Fragment>>& fragments = record.get_fragments_ref();

  std::map<int, std::vector<dataformats::WIBFrame*>> wibframes;

  for (auto& fragment : fragments) {
    auto id = fragment->get_element_id();
    auto element_id = id.element_id;
    int num_chunks = (fragment->get_size() - sizeof(dataformats::FragmentHeader)) / sizeof(dataformats::WIBFrame);
    std::vector<dataformats::WIBFrame*> tmp;
    for (int i = 0; i < num_chunks; ++i) {
      dataformats::WIBFrame* frame =
        reinterpret_cast<dataformats::WIBFrame*>(static_cast<char*>(fragment->get_data()) + (i * 464)); // NOLINT
      tmp.push_back(frame);
    }
    wibframes[element_id] = tmp;
  }

  return wibframes;
}

std::map<int, std::vector<dataformats::WIBFrame*>>
Decoder::decode(dataformats::TriggerRecord& record)
{
  return decodewib(record);
}

} // namespace dqm
} // namespace dunedaq

#endif // DQM_SRC_DECODER_HPP_
