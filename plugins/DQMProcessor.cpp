/**
 * @file DQMProcessor.cpp DQMProcessor Class Implementation
 *
 * See header for more on this class
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
// DQM includes
#include "dqm/Types.hpp"
#include "dqm/dqmprocessor/Nljs.hpp"
#include "dqm/dqmprocessor/Structs.hpp"

#include "DQMProcessor.hpp"
#include "Fourier.hpp"
#include "HistContainer.hpp"

#include "appfwk/DAQSource.hpp"
#include "dataformats/ComponentRequest.hpp"
#include "dataformats/Fragment.hpp"
#include "dataformats/GeoID.hpp"
#include "dataformats/TriggerRecord.hpp"
#include "dataformats/wib/WIBFrame.hpp"
#include "dfmessages/TriggerDecision.hpp"
#include "readout/ReadoutTypes.hpp"

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace dunedaq {
namespace dqm {

DQMProcessor::DQMProcessor(const std::string& name)
  : DAQModule(name)
{
  register_command("start", &DQMProcessor::do_start);
  register_command("conf", &DQMProcessor::do_configure);
  register_command("stop", &DQMProcessor::do_stop);
}

void
DQMProcessor::init(const data_t&)
{
  m_source.reset(new trigger_record_source_qt("trigger_record_q_dqm"));
  m_sink.reset(new trigger_decision_sink_qt("trigger_decision_q_dqm"));
  m_timesync_source.reset(new timesync_source_qt("time_sync_dqm_q"));
}

void
DQMProcessor::do_configure(const nlohmann::json& args)
{
  auto conf = args.get<dqmprocessor::Conf>();
  if (conf.mode == "debug" || conf.mode == "local processing") {
    m_running_mode = RunningMode::kLocalProcessing;
  } else if (conf.mode == "normal") {
    m_running_mode = RunningMode::kNormal;
  } else {
    TLOG() << "Invalid value for mode, supported values are 'debug', 'local processing' and 'normal'";
  }
  // m_source = std::unique_ptr<appfwk::DAQSource < std::unique_ptr<dataformats::TriggerRecord >>>
  // ("trigger_record_q_dqm"); m_sink = std::unique_ptr<appfwk::DAQSink < dfmessages::TriggerDecision >>
  // ("trigger_decision_q_dqm");
  m_standard_dqm = args.get<dqmprocessor::StandardDQM>();
  m_time_est = new timinglibs::TimestampEstimator(m_timesync_source, 1);
}

void
DQMProcessor::do_start(const nlohmann::json& args)
{
  m_run_marker.store(true);

  m_run_number.store(dataformats::run_number_t(
      args.at("run").get<dataformats::run_number_t>()));

  new std::thread(&DQMProcessor::RequestMaker, this);
}

void
DQMProcessor::do_stop(const data_t&)
{
  m_run_marker.store(false);
}

void
DQMProcessor::RequestMaker()
{

  // Helper struct with the necessary information about an instance
  struct AnalysisInstance
  {
    AnalysisModule* mod;
    double between_time;
    double default_unavailable_time;
    std::thread* running_thread;
    std::string name;
  };

  // For now only one link
  std::vector<dfmessages::GeoID> m_links;
  m_links.push_back({ dataformats::GeoID::SystemType::kTPC, 0, 0 });

  std::map<std::chrono::time_point<std::chrono::system_clock>, AnalysisInstance> map;

  std::unique_ptr<dataformats::TriggerRecord> element;

  // Instances of analysis modules
  HistContainer hist1s("hist1s", 256, 100, 0, 5000);
  HistContainer hist5s("hist5s", 256, 50, 0, 5000);
  HistContainer hist10s("hist10s", 256, 10, 0, 5000);
  FourierLink fourier10s("fourier10s", 0, 10, 100);

  // Initial tasks
  map[std::chrono::system_clock::now()] = {&hist1s, m_standard_dqm.histogram_how_often, m_standard_dqm.histogram_unavailable_time, nullptr, "Histogram every " + std::to_string(m_standard_dqm.histogram_how_often) + " s"};
  // map[std::chrono::system_clock::now()] = { &hist1s, 1, 1, nullptr, "Histogram every 1 s" };
  // map[std::chrono::system_clock::now()] = {&hist5s, 5, 1, nullptr, "Histogram every 5 s"};
  // map[std::chrono::system_clock::now()] = {&hist10s, 10, 1, nullptr, "Histogram every 10 s"};

  // Main loop, running forever
  while (m_run_marker) {

    auto fr = map.begin();
    if (fr == map.end()) {
      TLOG() << "Empty map! This should never happen!";
      break;
    }
    auto next_time = fr->first;
    AnalysisModule* algo = fr->second.mod;

    // Save pointer to delete the thread later
    std::thread* previous_thread = fr->second.running_thread;

    // TLOG() << "TIME: next_time" << next_time.time_since_epoch().count();

    // TLOG() << "MAIN LOOP" << " Instance of " << fr->second.between_time << " seconds" << (algo == &hist1s) << " " <<
    // (algo == &hist5s);

    // Sleep until the next time
    std::this_thread::sleep_until(next_time);

    // Make sure that the process is not running and a request can be made
    // otherwise we wait for more time
    if (algo->is_running()) {
      TLOG() << "ALGORITHM already running";
      map[std::chrono::system_clock::now() +
          std::chrono::milliseconds(static_cast<int>(fr->second.default_unavailable_time) * 1000)] = {
        algo, fr->second.between_time, fr->second.default_unavailable_time
      };
      map.erase(fr);
      continue;
    }
    // Before creating a request check that there
    // There has been a bug where the timestamp was retrieved before there were any timestamps
    // obtaining an invalid timestamps

    auto timestamp = m_time_est->get_timestamp_estimate();
    if (timestamp == dfmessages::TypeDefaults::s_invalid_timestamp) {
      continue;
    }

    // Now it's the time to do something
    auto request = CreateRequest(m_links);

    try {
      m_sink->push(request, m_sink_timeout);
    } catch (const ers::Issue &excpt) {
      TLOG() << "DQM: Unable to push to the request queue";
      continue;
    }

    // TLOG() << "Request pushed";

    // TLOG() << "Going to pop";
    try {
      m_source->pop(element, m_source_timeout);
    } catch (const ers::Issue &excpt) {
      TLOG() << "DQM: Unable to pop from the data queue";
      continue;
    }
    // TLOG() << "Element popped";

    std::thread* current_thread =
      new std::thread(&AnalysisModule::run, std::ref(*algo), std::ref(*element), m_running_mode);

    // Add a new entry for the current instance
    map[std::chrono::system_clock::now() +
        std::chrono::milliseconds(static_cast<int>(fr->second.between_time) * 1000)] = {
      algo, fr->second.between_time, fr->second.default_unavailable_time, current_thread, fr->second.name
    };

    // Delete the entry we just used and find the next one
    map.erase(fr);

    // Delete thread
    if (previous_thread != nullptr) {
      if (previous_thread->joinable()) {
        previous_thread->join();
        delete previous_thread; // TODO: rsipos -> Why is this a raw pointer on the thread? Move to unique_ptr.
      } else {
        // Should not be happening
      }
    }
  }
}

dfmessages::TriggerDecision
DQMProcessor::CreateRequest(std::vector<dfmessages::GeoID> m_links)
{
  auto timestamp = m_time_est->get_timestamp_estimate();
  dfmessages::TriggerDecision decision;

  static dataformats::trigger_number_t trigger_number = 1;

  decision.trigger_number = trigger_number++;
  decision.run_number = m_run_number;
  decision.trigger_timestamp = timestamp;
  decision.readout_type = dfmessages::ReadoutType::kMonitoring;

  int number_of_frames = 1;
  int window_size = number_of_frames * 25;

  for (auto& link : m_links) {
    // TLOG() << "ONE LINK";
    dataformats::ComponentRequest request;
    request.component = link;
    request.window_begin = timestamp;
    request.window_end = timestamp + window_size;

    decision.components.push_back(request);
  }

  return decision;
}

} // namespace dqm
} // namespace dunedaq

// Define the module
DEFINE_DUNE_DAQ_MODULE(dunedaq::dqm::DQMProcessor)
