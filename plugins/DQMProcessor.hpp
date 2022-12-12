/**
 * @file DQMProcessor.hpp DQMProcessor Class Interface
 *
 * DQMProcessor is a class that illustrates how to make a DUNE DAQ
 * module that interacts with another one. It does some printing to the screen
 * and prints the trigger number of a Fragment
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_PLUGINS_DQMPROCESSOR_HPP_
#define DQM_PLUGINS_DQMPROCESSOR_HPP_

#include "dqm/dqmprocessor/Nljs.hpp"
#include "dqm/dqmprocessor/Structs.hpp"
#include "dqm/dqmprocessorinfo/InfoNljs.hpp"

#include "dqm/ChannelMap.hpp"
#include "dqm/DQMFormats.hpp"

#include "appfwk/DAQModule.hpp"
#include "iomanager/IOManager.hpp"
#include "iomanager/Sender.hpp"
#include "iomanager/Receiver.hpp"
#include "daqdataformats/TriggerRecord.hpp"
#include "dfmessages/TriggerDecision.hpp"
#include "timinglibs/TimestampEstimator.hpp"
#include "ipm/Receiver.hpp"

#include "iomanager/queue/FollyQueue.hpp"

#include <atomic>
#include <chrono>
#include <string>
#include <vector>

namespace dunedaq::dqm {

class DQMProcessor : public dunedaq::appfwk::DAQModule
{

public:
  /**
   * @brief DQMProcessor Constructor
   * @param name Instance name for this DQMProcessor instance
   */
  explicit DQMProcessor(const std::string& name);

  DQMProcessor(const DQMProcessor&) = delete;            ///< DQMProcessor is not copy-constructible
  DQMProcessor& operator=(const DQMProcessor&) = delete; ///< DQMProcessor is not copy-assignable
  DQMProcessor(DQMProcessor&&) = delete;                 ///< DQMProcessor is not move-constructible
  DQMProcessor& operator=(DQMProcessor&&) = delete;      ///< DQMProcessor is not move-assignable

  void init(const data_t&) override;

  void do_print(const data_t&);
  void do_start(const data_t&);
  void do_drain_dataflow(const data_t&);
  void do_configure(const data_t&);

  void dispatch_timesync(dfmessages::TimeSync& timesyncmsg);
  void dispatch_trigger_record(std::unique_ptr<daqdataformats::TriggerRecord>& tr);

  void do_work();
  dfmessages::TriggerDecision create_readout_request(std::vector<dfmessages::SourceID>& m_sids, int number_of_frames, std::string& frontend_type);

  void dfrequest();

  void get_info(opmonlib::InfoCollector& ci, int /*level*/);

private:
  std::shared_ptr<std::atomic<bool>> m_run_marker;
  std::shared_ptr<iomanager::ReceiverConcept<std::unique_ptr<daqdataformats::TriggerRecord>>> m_receiver;
  std::shared_ptr<iomanager::SenderConcept<dfmessages::TriggerDecision>> m_sender;

  std::chrono::milliseconds m_sink_timeout{ 1000 };
  std::chrono::milliseconds m_source_timeout{ 1000 };

  // Configuration parameters
  dqmprocessor::StandardDQM m_raw_conf;
  dqmprocessor::StandardDQM m_std_conf;
  dqmprocessor::StandardDQM m_rms_conf;
  dqmprocessor::StandardDQM m_fourier_channel_conf;
  dqmprocessor::StandardDQM m_fourier_plane_conf;

  // DF configuration parameters
  int m_df_seconds {0};
  double m_df_offset {0};
  std::string m_df_algs;
  int m_df_num_frames {0};

  std::string m_df2dqm_connection;
  std::string m_dqm2df_connection;

  std::unique_ptr<timinglibs::TimestampEstimator> m_time_est;

  std::atomic<daqdataformats::run_number_t> m_run_number;

  std::string m_kafka_address;
  std::string m_kafka_topic;
  std::vector<int> m_link_idx;

  int m_clock_frequency;

  std::unique_ptr<std::thread> m_running_thread;

  std::atomic<int> m_request_count{ 0 };
  std::atomic<int> m_total_request_count{ 0 };
  std::atomic<int> m_data_count{ 0 };
  std::atomic<int> m_total_data_count{ 0 };
  std::atomic<uint64_t> m_received_timesync_count{ 0 }; // NOLINT(build/unsigned)

  std::string m_channel_map;

  iomanager::FollySPSCQueue<std::unique_ptr<daqdataformats::TriggerRecord>> dftrs{"FollyQueue", 100};

  std::string m_mode;
  std::string m_frontend_type;
  int m_readout_window_offset;

  DQMArgs m_dqm_args;
  DQMInfo m_dqm_info;
  int m_max_frames;

  // Constants used in DQMProcessor.cpp
  static constexpr int m_channel_map_delay {2};                // How much time in s to wait until running the channel map
  static constexpr int m_offset_from_channel_map {10};         // How much time in s to wait after the channel map has been filled to run the other algorithms
  static constexpr int m_sleep_time {300};                     // How much time in ms to sleep between checking if the run has been stopped
  static constexpr int m_sleep_time_df {300};                  // How much time in ms to sleep between checking if a TR from DF has arrived
  static constexpr int m_timesync_check {100};                 // How much time in ms to wait for correct timesync messages
};

} // namespace dunedaq::dqm

#endif // DQM_PLUGINS_DQMPROCESSOR_HPP_
