/**
 * @file ChannelStream.hpp Implementation of a container of Hist objects
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_CHANNELSTREAM_HPP_
#define DQM_SRC_CHANNELSTREAM_HPP_

// DQM
#include "dqm/AnalysisModule.hpp"
#include "ChannelMap.hpp"
#include "dqm/Constants.hpp"
#include "dqm/Decoder.hpp"
#include "dqm/Exporter.hpp"
#include "dqm/Issues.hpp"
#include "dqm/Pipeline.hpp"
#include "dqm/DQMFormats.hpp"
#include "dqm/DQMLogging.hpp"
#include "dqm/FormatUtils.hpp"

#include "daqdataformats/TriggerRecord.hpp"

#include <functional>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace dunedaq::dqm {

template<class T, class I>
class ChannelStream : public AnalysisModule
{

public:
  ChannelStream(std::string name,
                int nhist,
                std::vector<int>& link_idx,
                std::function<std::vector<I>(std::vector<T>&, int, int)> function);

  void run(std::shared_ptr<daqdataformats::TriggerRecord> record,
      DQMArgs& args, DQMInfo& info) override;

  template <class R>
  void run_(std::shared_ptr<daqdataformats::TriggerRecord> record,
       DQMArgs& args, DQMInfo& info);

  void transmit(const std::string& kafka_address,
                std::shared_ptr<ChannelMap>& cmap,
                const std::string& topicname,
                int run_num);

  void clean();
  void fill(int ch, int link, I value);
  int get_local_index(int ch, int link);

private:
  std::string m_name;
  std::vector<T> histvec;
  int m_size;
  std::map<int, int> m_index;

  std::function<std::vector<I>(std::vector<T>&, int, int)> m_function;
};

template <class T, class I>
ChannelStream<T, I>::ChannelStream(std::string name,
            int nchannels,
            std::vector<int>& link_idx,
            std::function<std::vector<I>(std::vector<T>&, int, int)> function)
  : m_name(name)
  , m_size(nchannels)
  , m_function(function)
{
  for (int i = 0; i < m_size; ++i) {
    histvec.emplace_back(T());
  }
  int channels = 0;
  for (size_t i = 0; i < link_idx.size(); ++i) {
    m_index[link_idx[i]] = channels;
    channels += CHANNELS_PER_LINK;
  }
}

template <class T, class I>
void
ChannelStream<T, I>::run(std::shared_ptr<daqdataformats::TriggerRecord> record,
                   DQMArgs& args, DQMInfo& info)
{
  TLOG(TLVL_WORK_STEPS) << "Running "<< m_name << " with frontend_type = " << args.frontend_type;
  auto start = std::chrono::steady_clock::now();
  auto frontend_type = args.frontend_type;
  if (frontend_type == "wib") {
    set_is_running(true);
    run_<detdataformats::wib::WIBFrame>(std::move(record), args, info);
    set_is_running(false);
  }
  else if (frontend_type == "wib2") {
    set_is_running(true);
    run_<detdataformats::wib2::WIB2Frame>(std::move(record), args, info);
    set_is_running(false);
  }
  auto stop = std::chrono::steady_clock::now();


  // else if (frontend_type == "tde") {
  //   set_is_running(true);
  //   auto ret = run_<detdataformats::wib::WIBFrame>(std::move(record), map, kafka_address);
  //   set_is_running(false);
  //   return ret;
  // }
  info.std_time_taken.store(std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count());
  info.std_times_run++;
}

template <class T, class I>
template <class R>
void
ChannelStream<T, I>::run_(std::shared_ptr<daqdataformats::TriggerRecord> record,
                DQMArgs& args, DQMInfo& info)
{
  auto map = args.map;

  auto frames = decode<R>(record, args.max_frames);
  auto pipe = Pipeline<R>({"remove_empty", "check_empty", "check_timestamps_aligned"});
  bool valid_data = pipe(frames);
  if (!valid_data) {
    return;
  }

  // Get all the keys
  std::vector<int> keys;
  for (auto& [key, value] : frames) {
    keys.push_back(key);
  }

  for (const auto& [key, vec] : frames) {
    for (const auto& fr : vec) {
      for (int ich = 0; ich < CHANNELS_PER_LINK; ++ich) {
        fill(ich, key, get_adc<R>(fr, ich));
      }
    }
  }

  transmit(args.kafka_address,
           map,
           args.kafka_topic,
           record->get_header_ref().get_run_number());
  clean();
}

template <class T, class I>
void
ChannelStream<T, I>::transmit(const std::string& kafka_address,
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
    output << "\"algorithm\": \"" << m_name << "\"";
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
    std::vector<I> values;
    for (auto& [offch, pair] : map) {
      int link = pair.first;
      int ch = pair.second;
      for (const auto& entry : m_function(histvec, ch, link)) {
        values.push_back(entry);
      }
    }
    bytes = serialization::serialize(values, serialization::kMsgPack);
    for (auto& b : bytes) {
      output << b;
    }
    // Split the message if it's too big
    // Kafka doesn't let it go through when it's very close to the limit
    int max_size = .99 * 1e6;
    std::string str = output.str();
    auto index = str.find("}\n\n\n");
    std::string header = str.substr(0, index);
    // Assume the number of parts is at most double digits, then the size of the new part of the header
    // is at most 31 bytes
    int parts = (str.size() - header.size() - 4) / (max_size - header.size() - 4 - 31) + ((str.size() - header.size() - 4) % (max_size - header.size() - 4 -31) > 0);
    TLOG_DEBUG(TLVL_WORK_STEPS) << "Splitting message in " << parts << " parts";
    if (parts > 99) {
      return;
    }
    for (int i = 0; i < parts; ++i) {
      std::string newheader = header;
      newheader += ",\"part\":\"" + std::to_string(i+1) + "\",";
      newheader += "\"total_parts\":\"" + std::to_string(parts) + "\"";
      std::string body = str.substr(index + 4 + ((max_size - header.size() - 4 - 31) * i), max_size - header.size() - 4 - 31);
      TLOG() << "body.size() = " << body.size();
      KafkaExport(kafka_address, newheader + "}\n\n\n" + body, topicname);
    }
  }
}

template <class T, class I>
void
ChannelStream<T, I>::clean()
{
  for (int ich = 0; ich < m_size; ++ich) {
    histvec[ich].clean();
  }
}

template <class T, class I>
void
ChannelStream<T, I>::fill(int ch, int link, I value)
{
  histvec[ch + m_index[link]].fill(value);
}

template <class T, class I>
int
ChannelStream<T, I>::get_local_index(int ch, int link)
{
  return ch + m_index[link];
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_CHANNELSTREAM_HPP_
