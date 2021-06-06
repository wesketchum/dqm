/**
 * @file DataReceiver.hpp DataReceiver Class Interface
 *
 * DataReceiver is a class that illustrates how to make a DUNE DAQ
 * module that interacts with another one. It does some printing to the screen
 * and prints the trigger number of a Fragment
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include <atomic>
#include <chrono>
#include <utility>

#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQSource.hpp"
#include "appfwk/DAQSink.hpp"

#include "dfmessages/TriggerDecision.hpp"

#include "dataformats/TriggerRecord.hpp"


namespace dunedaq::dqm {

class DataReceiver : public dunedaq::appfwk::DAQModule
{

public:
 /**
 * @brief DataReceiver Constructor
 * @param name Instance name for this DataReceiver instance
 */
  explicit DataReceiver(const std::string& name);

  DataReceiver(const DataReceiver&) =
    delete; ///< DataReceiver is not copy-constructible
  DataReceiver& operator=(const DataReceiver&) =
    delete; ///< DataReceiver is not copy-assignable
  DataReceiver(DataReceiver&&) =
    delete; ///< DataReceiver is not move-constructible
  DataReceiver& operator=(DataReceiver&&) =
    delete; ///< DataReceiver is not move-assignable

  void init(const data_t& ) override;

  void do_print(const data_t&);
  void do_start(const data_t&);
  void do_stop(const data_t&);
  void do_configure(const data_t&);

  std::atomic<bool> m_run_marker;

  void RequestMaker();
  dfmessages::TriggerDecision CreateRequest(std::vector<dfmessages::GeoID> m_links);

private:

  using trigger_record_source_qt = appfwk::DAQSource < std::unique_ptr<dataformats::TriggerRecord>>;
  std::unique_ptr<trigger_record_source_qt> m_source;
  using trigger_decision_sink_qt = appfwk::DAQSink <dfmessages::TriggerDecision>;
  std::unique_ptr<trigger_decision_sink_qt> m_sink;

  std::chrono::milliseconds m_sink_timeout{1000};
  std::chrono::milliseconds m_source_timeout{1000};

  std::string m_running_mode;

};


} // namespace dunedaq::dqm


