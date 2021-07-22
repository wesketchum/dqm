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
#include <vector>
#include <memory>

namespace dunedaq {
namespace dqm {

class Decoder
{
public:
  std::vector<dataformats::WIBFrame*> decode(dunedaq::dataformats::TriggerRecord& record);
};

std::vector<dataformats::WIBFrame*>
decodewib(dataformats::TriggerRecord& record)
{
  std::vector<std::unique_ptr<dataformats::Fragment>>& fragments = record.get_fragments_ref();

  std::vector<dataformats::WIBFrame*> wib_frames;

  for (auto& fragment : fragments) {
    // There is some debugging code for timestamps, should be improved
    int num_chunks = (fragment->get_size() - sizeof(dataformats::FragmentHeader)) / sizeof(dataformats::WIBFrame);
    // TLOG() << "Number of chunks being decoded: " << num_chunks;
    // uint64_t mintimestamp = ULONG_LONG_MAX;
    // uint64_t maxtimestamp = 0;
    for (int i = 0; i < num_chunks; ++i) {
      dataformats::WIBFrame* frame =
        reinterpret_cast<dataformats::WIBFrame*>(static_cast<char*>(fragment->get_data()) + (i * 464)); // NOLINT
      // uint64_t timestamp = frame->get_wib_header()->get_timestamp();
      // mintimestamp = std::min(mintimestamp, timestamp);
      // maxtimestamp = std::max(maxtimestamp, timestamp);
      wib_frames.push_back(frame);
    }
    // TLOG_DEBUG(TLVL_BOOKKEEPING) << "Min timestamp is " << mintimestamp << ", max timestamp is << maxtimestamp";
  }

  return wib_frames;
}

std::vector<dataformats::WIBFrame*>
Decoder::decode(dataformats::TriggerRecord& record)
{
  return decodewib(record);
}

} // namespace dqm
} // namespace dunedaq

#endif // DQM_SRC_DECODER_HPP_
