/**
 * @file Decoder.hpp Implementation of decoders used in DQM analysis modules
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_DECODER_HPP_
#define DQM_SRC_DECODER_HPP_

#include "daqdataformats/Fragment.hpp"
#include "daqdataformats/TriggerRecord.hpp"

#include "ers/Issue.hpp"
#include "dqm/Issues.hpp"
#include "dqm/DQMLogging.hpp"

#include <map>
#include <memory>
#include <vector>
#include <string>

namespace dunedaq {
namespace dqm {

using logging::TLVL_WORK_STEPS;

template<class T>
std::map<int, std::vector<T*>>
decode(std::shared_ptr<daqdataformats::TriggerRecord> record, int max_frames) {
  const std::vector<std::unique_ptr<daqdataformats::Fragment>>& fragments = record->get_fragments_ref();

  std::map<int, std::vector<T*>> frames;

  std::string types_str;
  std::string sizes_str;
  for (const auto& fragment : fragments) {
    types_str += std::to_string(static_cast<uint32_t>(fragment->get_fragment_type())) + " ";
    sizes_str += std::to_string(fragment->get_size()) + " ";
  }
  TLOG_DEBUG(TLVL_WORK_STEPS) << "Decoding TriggerRecord with " << fragments.size() << " fragments with types "
                              << types_str << "and sizes " << sizes_str;

  for (const auto& fragment : fragments) {
    if (fragment->get_fragment_type() != daqdataformats::FragmentType::kProtoWIB &&
        fragment->get_fragment_type() != daqdataformats::FragmentType::kWIB &&
        fragment->get_fragment_type() != daqdataformats::FragmentType::kTDE_AMC) {
      continue;
    }
    auto id = fragment->get_element_id();
    auto element_id = id.id;
    int num_chunks =
      (fragment->get_size() - sizeof(daqdataformats::FragmentHeader)) / sizeof(T);
    TLOG() << "size is " << fragment->get_size();
    TLOG() << "num_chunks = " << num_chunks;
    std::vector<T*> tmp;
    // Don't put a limit if max_frames = 0
    if (max_frames > 0) {
      num_chunks = std::min(max_frames, num_chunks);
    }
    for (int i = 0; i < num_chunks; ++i) {
      T* frame = reinterpret_cast<T*>( // NOLINT
      static_cast<char*>(fragment->get_data()) + (i * sizeof(T)));
      tmp.push_back(frame);
    }
    frames[element_id] = tmp;
  }

  return frames;
}

} // namespace dqm
} // namespace dunedaq

#endif // DQM_SRC_DECODER_HPP_
