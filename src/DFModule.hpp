/**
 * @file DFModule.hpp Class for running algorithms on the TRs from DF
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_DFMODULE_HPP_
#define DQM_SRC_DFMODULE_HPP_

// DQM
#include "dqm/AnalysisModule.hpp"
#include "ChannelMap.hpp"
#include "Constants.hpp"
#include "dqm/Issues.hpp"

#include "RMSModule.hpp"
#include "STDModule.hpp"
#include "CounterModule.hpp"
#include "FourierContainer.hpp"

#include "daqdataformats/TriggerRecord.hpp"

#include <string>

namespace dunedaq::dqm {

class DFModule : public AnalysisModule
{

public:
  DFModule(bool enable_raw, bool enable_rms, bool enable_std, bool enable_fourier_channel, bool enable_fourier_sum_plane,
           int clock_frequency, std::vector<int>& ids, int num_frames, std::string& frontend_type);

  bool m_enable_raw, m_enable_rms, m_enable_std, m_enable_fourier_channel, m_enable_fourier_plane;
  int m_clock_frequency;
  std::unique_ptr<daqdataformats::TriggerRecord>
  run(std::unique_ptr<daqdataformats::TriggerRecord> record,
      DQMArgs& args);

private:

  std::shared_ptr<RMSModule> m_rms;
  std::shared_ptr<STDModule> m_std;
  std::shared_ptr<CounterModule> m_raw;
  std::shared_ptr<FourierContainer> m_fourier_channel, m_fourier_plane;

  std::vector<int> m_ids;

  int m_num_frames;

};

DFModule::DFModule(bool enable_raw, bool enable_rms, bool enable_std, bool enable_fourier_channel, bool enable_fourier_sum_plane,
                   int clock_frequency, std::vector<int>& ids, int num_frames, std::string& frontend_type) :
    m_enable_raw(enable_raw),
    m_enable_rms(enable_rms),
    m_enable_std(enable_std),
    m_enable_fourier_channel(enable_fourier_channel),
    m_enable_fourier_plane(enable_fourier_sum_plane),
    m_clock_frequency(clock_frequency),
    m_ids(ids),
    m_num_frames(num_frames)

{
  if (m_enable_raw) {
    m_raw = std::make_shared<CounterModule>("raw", CHANNELS_PER_LINK * m_ids.size(), m_ids);
  }
  if (m_enable_rms) {
    m_rms = std::make_shared<RMSModule>("rms", CHANNELS_PER_LINK * m_ids.size(), m_ids);
  }
  if (m_enable_std) {
    m_std = std::make_shared<STDModule>("std", CHANNELS_PER_LINK * m_ids.size(), m_ids);
  }
  if (m_enable_fourier_channel) {
    m_fourier_channel = std::make_shared<FourierContainer>("fft_display",
                                                    CHANNELS_PER_LINK * m_ids.size(),
                                                    m_ids,
                                                   1. / m_clock_frequency * (strcmp(frontend_type.c_str(), "wib") ? 32 : 25),
                                                    m_num_frames);
  }
  if (m_enable_fourier_plane) {
    m_fourier_plane = std::make_shared<FourierContainer>("fft_sums_display",
                                                       4,
                                                       m_ids,
                                                       1. / m_clock_frequency * (strcmp(frontend_type.c_str(), "wib") ? 32 : 25),
                                                       m_num_frames,
                                                       true);
  }
}

std::unique_ptr<daqdataformats::TriggerRecord>
DFModule::run(std::unique_ptr<daqdataformats::TriggerRecord> record,
              DQMArgs& args)
{
  set_is_running(true);
  auto run_mark = args.run_mark;
  auto map = args.map;
  auto frontend_type = args.frontend_type;
  auto kafka_address = args.kafka_address;

  std::vector<std::shared_ptr<AnalysisModule>> list {m_raw, m_std, m_fourier_channel, m_fourier_plane};
  std::vector<bool> will_run {m_enable_raw, m_enable_std, m_enable_fourier_channel, m_enable_fourier_plane};

  for(size_t i=0; i < list.size(); ++i) {
    if (!will_run[i]) continue;
    if (!run_mark) {
      set_is_running(false);
      return std::move(record);
    }
    DQMArgs args {run_mark, map, frontend_type, kafka_address};
    record = list[i]->run(std::move(record), args);
  }

  set_is_running(false);
  return std::move(record);
}


} // namespace dunedaq::dqm

#endif // DQM_SRC_DFMODULE_HPP_
