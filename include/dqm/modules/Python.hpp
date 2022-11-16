/**
 * @file PythonModule.hpp Implementation of a container of Fourier objects
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_MODULES_PYTHON_HPP_
#define DQM_INCLUDE_DQM_MODULES_PYTHON_HPP_

// DQM
#include "dqm/AnalysisModule.hpp"
#include "dqm/ChannelMap.hpp"
#include "dqm/Constants.hpp"
#include "dqm/Decoder.hpp"
#include "dqm/Exporter.hpp"
#include "dqm/algs/Fourier.hpp"
#include "dqm/Issues.hpp"
#include "dqm/DQMFormats.hpp"
#include "dqm/DQMLogging.hpp"
#include "dqm/PythonUtils.hpp"

#include "daqdataformats/TriggerRecord.hpp"

#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/python.hpp>
#include <boost/python/errors.hpp>
#include <boost/python/numpy.hpp>
#include <pybind11/stl.h>

namespace dunedaq::dqm {

namespace p = boost::python;
namespace np = boost::python::numpy;

using logging::TLVL_WORK_STEPS;

class PythonModule : public AnalysisModule
{
  std::string m_name;
  std::vector<Fourier> fouriervec;
  size_t m_size;
  int m_npoints;
  std::map<int, int> m_index;
  bool m_global_mode;

public:
  PythonModule(std::string name);

  void run(std::shared_ptr<daqdataformats::TriggerRecord> record,
      DQMArgs& args, DQMInfo& info) override;

  template <class T>
  void run_(std::shared_ptr<daqdataformats::TriggerRecord> record,
       DQMArgs& args, DQMInfo& info);

  void transmit_global(const std::string &kafka_address,
                       std::shared_ptr<ChannelMap> cmap,
                       const std::string& topicname,
                       int run_num);
  void clean();
  void fill(int ch, double value);
  void fill(int ch, int link, double value);
  int get_local_index(int ch, int link);
};

PythonModule::PythonModule(std::string name)
  : m_name(name)
{
}

template <class T>
void
PythonModule::run_(std::shared_ptr<daqdataformats::TriggerRecord> record,
                       DQMArgs& args, DQMInfo& info)
{
  auto start = std::chrono::steady_clock::now();
  auto map = args.map;
  auto frames = decode<T>(record, args.max_frames);
  auto pipe = Pipeline<T>({"remove_empty", "check_empty", "make_same_size", "check_timestamps_aligned"});
  bool valid_data = pipe(frames);
  if (!valid_data) {
    set_is_running(false);
    return;
  }

  TLOG() << "Calling python";
  p::object module;
  try {
    module = p::import("dqm_std");
  } catch (p::error_already_set& e) {
    std::cout << "Unable to import module" << std::endl;
  }

  typedef std::map<int, std::vector<T*>> mapt;
  auto channels = MapItem<mapt>::get_channels(frames, map);
  auto planes = MapItem<mapt>::get_planes(frames, map);

  p::object f = module.attr("func");
  f(frames, channels, planes);
  TLOG() << "Calling python done";

  // info.fourier_plane_time_taken.store(std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count());
  // info.fourier_plane_times_run++;

}


void
PythonModule::run(std::shared_ptr<daqdataformats::TriggerRecord> record,
                      DQMArgs& args, DQMInfo& info)
{
  TLOG(TLVL_WORK_STEPS) << "Running Fourier Transform with frontend_type = " << args.frontend_type;
  auto frontend_type = args.frontend_type;
  auto run_mark = args.run_mark;
  auto map = args.map;
  auto kafka_address = args.kafka_address;
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
}

// void
// PythonModule::transmit(const std::string& kafka_address,
//                            std::shared_ptr<ChannelMap> cmap,
//                            const std::string& topicname,
//                            int run_num,
//                            time_t timestamp)
// {

  // // Placeholders
  // std::string dataname = m_name;
  // std::string partition = getenv("DUNEDAQ_PARTITION");
  // std::string app_name = getenv("DUNEDAQ_APPLICATION_NAME");
  // std::string datasource = partition + "_" + app_name;

  // // One message is sent for every plane
  // auto channel_order = cmap->get_map();
  // for (auto& [plane, map] : channel_order) {
  //   std::stringstream output;
  //   output << "{";
  //   output << "\"source\": \"" << datasource << "\",";
  //   output << "\"run_number\": \"" << run_num << "\",";
  //   output << "\"partition\": \"" << partition << "\",";
  //   output << "\"app_name\": \"" << app_name << "\",";
  //   output << "\"plane\": \"" << plane << "\",";
  //   output << "\"algorithm\": \"" << "std" << "\"";
  //   output << "}\n\n\n";
  //   std::vector<float> freqs = fouriervec[0].get_frequencies();
  //   auto bytes = serialization::serialize(freqs, serialization::kMsgPack);
  //   for (auto& b : bytes) {
  //     output << b;
  //   }
  //   output << "\n\n\n";
  //   std::vector<float> values;
  //   for (auto& [offch, pair] : map) {
  //     int link = pair.first;
  //     int ch = pair.second;
  //     values.push_back(histvec[get_local_index(ch, link)].std());
  //   }
  //   bytes = serialization::serialize(values, serialization::kMsgPack);
  //   for (auto& b : bytes) {
  //     output << b;
  //   }
  //   TLOG_DEBUG(5) << "Size of the message in bytes: " << output.str().size();
  //   KafkaExport(kafka_address, output.str(), topicname);
  // }

  // auto freq = fouriervec[0].get_frequencies();
  // // One message is sent for every plane
  // auto channel_order = cmap->get_map();
  // for (auto& [plane, map] : channel_order) {
  //   std::stringstream output;
  //   output << datasource << ";" << dataname << ";" << run_num << ";" << subrun << ";" << event << ";" << timestamp
  //          << ";" << metadata << ";" << partition << ";" << app_name << ";" << 0 << ";" << plane << ";";
  //   for (auto& [offch, pair] : map) {
  //     output << offch << " ";
  //   }
  //   output << "\n";
  //   for (size_t i = 0; i < freq.size(); ++i) {
  //     output << freq[i] << "\n";
  //     for (auto& [offch, pair] : map) {
  //       int link = pair.first;
  //       int ch = pair.second;
  //       output << fouriervec[get_local_index(ch, link)].get_transform_at(i) << " ";
  //     }
  //     output << "\n";
  //   }
  //   TLOG_DEBUG(5) << "Size of the message in bytes: " << output.str().size();
  //   KafkaExport(kafka_address, output.str(), topicname);
  // }

  // clean();
// }

void
PythonModule::transmit_global(const std::string& kafka_address,
                                  std::shared_ptr<ChannelMap>,
                                  const std::string& topicname,
                                  int run_num)
{
  // Placeholders
  std::string dataname = m_name;
  std::string partition = getenv("DUNEDAQ_PARTITION");
  std::string app_name = getenv("DUNEDAQ_APPLICATION_NAME");
  std::string datasource = partition + "_" + app_name;

  // One message is sent for every plane
  for (int plane = 0; plane < 4; plane++) {
    std::stringstream output;
    output << "{";
    output << "\"source\": \"" << datasource << "\",";
    output << "\"run_number\": \"" << run_num << "\",";
    output << "\"partition\": \"" << partition << "\",";
    output << "\"app_name\": \"" << app_name << "\",";
    output << "\"plane\": \"" << plane << "\",";
    output << "\"algorithm\": \"" << "fourier_plane" << "\"";
    output << "}\n\n\n";
    std::vector<double> freqs = fouriervec[plane].get_frequencies();
    auto bytes = serialization::serialize(freqs, serialization::kMsgPack);
    for (auto& b : bytes) {
      output << b;
    }
    output << "\n\n\n";
    auto tmp = fouriervec[plane].get_transform();
    std::vector<double> values;
    values.reserve(tmp.size());
    for (const auto& v : tmp) {
      values.push_back(abs(v));
    }
    bytes = serialization::serialize(values, serialization::kMsgPack);
    for (auto& b : bytes) {
      output << b;
    }
    TLOG_DEBUG(5) << "Size of the message in bytes: " << output.str().size();
    KafkaExport(kafka_address, output.str(), topicname);
  }

  clean();
}

void
PythonModule::clean()
{
  for (size_t ich = 0; ich < m_size; ++ich) {
    fouriervec[ich].clean();
  }
}

void
PythonModule::fill(int ch, double value)
{
  fouriervec[ch].fill(value);
}

void
PythonModule::fill(int ch, int link, double value)
{
  fouriervec[ch + m_index[link]].fill(value);
}

int
PythonModule::get_local_index(int ch, int link)
{
  return ch + m_index[link];
}

} // namespace dunedaq::dqm

#endif // DQM_INCLUDE_DQM_MODULES_PYTHON_HPP_
