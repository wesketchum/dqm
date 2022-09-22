
/**
 * @file CounterModule.hpp Implementation of a container of Hist objects
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_CounterModule_HPP_
#define DQM_SRC_CounterModule_HPP_

// DQM
#include "dqm/AnalysisModule.hpp"
#include "ChannelMap.hpp"
#include "Constants.hpp"
#include "Decoder.hpp"
#include "Exporter.hpp"
#include "dqm/Issues.hpp"
#include "dqm/algs/Counter.hpp"
#include "dqm/FormatUtils.hpp"
#include "dqm/Pipeline.hpp"

#include "daqdataformats/TriggerRecord.hpp"
#include "detdataformats/tde/TDE16Frame.hpp"

#include <cstdlib>
#include <map>
#include <string>
#include <vector>

namespace dunedaq::dqm {

class CounterModule : public AnalysisModule
{

public:
   CounterModule(std::string name,
            int nchannels,
            std::vector<int>& link_idx);

  std::unique_ptr<daqdataformats::TriggerRecord>
  run(std::unique_ptr<daqdataformats::TriggerRecord> record,
      DQMArgs& args, DQMInfo& info);

  template <class T>
  std::unique_ptr<daqdataformats::TriggerRecord>
  run_(std::unique_ptr<daqdataformats::TriggerRecord> record,
       DQMArgs& args, DQMInfo& info);

  // std::unique_ptr<daqdataformats::TriggerRecord>
  // run_tdeframe(std::unique_ptr<daqdataformats::TriggerRecord> record,
  //               std::shared_ptr<ChannelMap>& map,
  //               const std::string& kafka_address = "");
  void transmit(const std::string& kafka_address,
                std::shared_ptr<ChannelMap>& map,
                const std::string& topicname,
                int run_num);

  void clean();
  void fill(int ch, double value);
  void fill(int ch, int link, double value);
  int get_local_index(int ch, int link);

private:
  std::string m_name;
  std::vector<Counter> histvec;
  int m_size;
  std::map<int, int> m_index;
};


 CounterModule:: CounterModule(std::string name,
                             int nchannels,
                             std::vector<int>& link_idx
                     )
  : m_name(name)
  , m_size(nchannels)
{
  for (int i = 0; i < m_size; ++i) {
    histvec.emplace_back(Counter());
  }
  int channels = 0;
  for (size_t i = 0; i < link_idx.size(); ++i) {
    m_index[link_idx[i]] = channels;
    channels += CHANNELS_PER_LINK;
  }
}

// std::unique_ptr<daqdataformats::TriggerRecord>
//  CounterModule::run_tdeframe(std::unique_ptr<daqdataformats::TriggerRecord> record,
//                              std::shared_ptr<ChannelMap>& map,
//                              const std::string& kafka_address)
// {
//   TLOG() << "Running run_tdeframe";
//   auto wibframes = decode<detdataformats::tde::TDE16Frame>(*record);
//   return std::move(record);
// }

template <class T>
std::unique_ptr<daqdataformats::TriggerRecord>
 CounterModule::run_(std::unique_ptr<daqdataformats::TriggerRecord> record,
                     DQMArgs& args, DQMInfo& info)
{
  auto map = args.map;

  auto frames = decode<T>(*record, args.max_frames);
  auto pipe = Pipeline<T>({"remove_empty", "check_empty", "make_same_size", "check_timestamp_aligned"});
  pipe(frames);

  // Get all the keys
  std::vector<int> keys;
  for (auto& [key, value] : frames) {
    keys.push_back(key);
  }

  uint64_t min_timestamp = 0; // NOLINT(build/unsigned)
  // We run over all links until we find one that has a non-empty vector of frames
  for (auto& key : keys) {
    if (!frames[key].empty()) {
      min_timestamp = get_timestamp<T>(frames[key].front());
      break;
    }
  }
  // Check that all the frames vectors have the same size, if not, something
  // bad has happened, for now don't do anything
  // auto size = frames.begin()->second.size();
  // for (auto& vec : frames) {
  //   if (vec.second.size() != size) {
  //     ers::error(InvalidData(ERS_HERE, "the size of the vector of frames is different for each link"));
  //     set_is_running(false);
  //     return std::move(record);
  //   }
  // }


  // Main loop
  // If only the mean and rms are to be sent all frames are processed
  // and at the end the result is transmitted
  // If it's in the raw display mode then the result is saved for
  // every frame and sent at the end

  // Fill for every frame, outer loop so it is done frame by frame
  // This is needed for sending frame by frame
  // The order does not matter for the mean and RMS
  for (size_t ifr = 0; ifr < frames[keys[0]].size(); ++ifr) {
    // Fill for every link
    for (size_t ikey = 0; ikey < keys.size(); ++ikey) {
      auto fr = frames[keys[ikey]][ifr];

      for (int ich = 0; ich < CHANNELS_PER_LINK; ++ich) {
        fill(ich, keys[ikey], get_adc<T>(fr, ich));
      }
    }
  }
  transmit(args.kafka_address,
           map,
           args.kafka_topic,
           record->get_header_ref().get_run_number());
  clean();

  return std::move(record);

}



std::unique_ptr<daqdataformats::TriggerRecord>
 CounterModule::run(std::unique_ptr<daqdataformats::TriggerRecord> record,
                    DQMArgs& args, DQMInfo& info)
{
  auto start = std::chrono::steady_clock::now();
  auto frontend_type = args.frontend_type;
  if (frontend_type == "wib") {
    set_is_running(true);
    auto ret = run_<detdataformats::wib::WIBFrame>(std::move(record), args, info);
    set_is_running(false);
  }
  else if (frontend_type == "wib2") {
    set_is_running(true);
    auto ret = run_<detdataformats::wib2::WIB2Frame>(std::move(record), args, info);
    set_is_running(false);
  }
  auto stop = std::chrono::steady_clock::now();
  // else if (frontend_type == "tde") {
  //   set_is_running(true);
  //   auto ret = run_<detdataformats::wib::WIBFrame>(std::move(record), map, kafka_address);
  //   set_is_running(false);
  //   return ret;
  // }
  info.info["raw_time_taken"].store(std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count());
  info.info["raw_times_run"].store(info.info["raw_times_run"].load() + 1);
  return record;
}

void
 CounterModule::transmit(const std::string& kafka_address,
                    std::shared_ptr<ChannelMap>& cmap,
                    const std::string& topicname,
                    int run_num)
{
  // Placeholders
  std::string dataname = m_name;
  std::string partition = getenv("DUNEDAQ_PARTITION");
  std::string app_name = getenv("DUNEDAQ_APPLICATION_NAME");
  std::string datasource = partition + "_" + app_name;

  // One message is sent for every plane
  auto channel_order = cmap->get_map();
  for (auto& [plane, map] : channel_order) {
    std::stringstream output;
    output << "{";
    output << "\"source\": \"" << datasource << "\",";
    output << "\"run_number\": \"" << run_num << "\",";
    output << "\"partition\": \"" << partition << "\",";
    output << "\"app_name\": \"" << app_name << "\",";
    output << "\"plane\": \"" << plane << "\",";
    output << "\"algorithm\": \"" << "raw" << "\"";
    output << "}\n\n\n";
    std::vector<int> channels;
    for (auto& [offch, pair] : map) {
      channels.push_back(offch);
    }
    auto bytes = serialization::serialize(channels, serialization::kMsgPack);
    for (auto& b : bytes) {
      output << b;
    }
    output << "\n\n\n";
    std::vector<int> values;
    for (auto& [offch, pair] : map) {
      int link = pair.first;
      int ch = pair.second;
      for (const auto& entry : histvec[get_local_index(ch, link)].count) {
        values.push_back(entry);
      }
    }
    bytes = serialization::serialize(values, serialization::kMsgPack);
    for (auto& b : bytes) {
      output << b;
    }
    TLOG_DEBUG(5) << "Size of the message in bytes: " << output.str().size();
    KafkaExport(kafka_address, output.str(), topicname);
  }
}

void
 CounterModule::clean()
{
  for (int ich = 0; ich < m_size; ++ich) {
    histvec[ich].clean();
  }
}

void
 CounterModule::fill(int ch, double value)
{
  histvec[ch].fill(value);
}

void
 CounterModule::fill(int ch, int link, double value)
{
  histvec[ch + m_index[link]].fill(value);
}

int
 CounterModule::get_local_index(int ch, int link)
{
  return ch + m_index[link];
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_CounterModule_HPP_