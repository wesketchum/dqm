/**
 * @file DFModule.hpp Implementation of a container of Hist objects
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_DFMODULE_HPP_
#define DQM_SRC_DFMODULE_HPP_

// DQM
#include "AnalysisModule.hpp"
#include "SortedChannelMap.hpp"
#include "Constants.hpp"
#include "dqm/DQMIssues.hpp"
#include "HistContainer.hpp"
#include "FourierContainer.hpp"

#include "daqdataformats/TriggerRecord.hpp"

#include <string>

namespace dunedaq::dqm {

class DFModule : public AnalysisModule
{

public:
  DFModule(bool enable_hist, bool enable_mean_rms, bool enable_fourier, bool enable_fourier_sum,
           std::vector<daqdataformats::GeoID>& ids);

  bool m_enable_hist, m_enable_mean_rms, m_enable_fourier, m_enable_fourier_sum;

private:

  std::shared_ptr<HistContainer> m_hist, m_mean_rms;
  std::shared_ptr<FourierContainer> m_fourier, m_fourier_sum;

  std::vector<daqdataformats::GeoID> m_ids;

};

DFModule::DFModule(bool enable_hist, bool enable_mean_rms, bool enable_fourier, bool enable_fourier_sum,
                   std::vector<daqdataformats::GeoID>& ids) :
    m_enable_hist(enable_hist),
    m_enable_mean_rms(enable_mean_rms),
    m_enable_fourier(enable_fourier),
    m_enable_fourier_sum(enable_fourier_sum),
    m_ids(ids)

{
  if (m_enable_hist) {
    m_hist.reset(std::make_shared<HistContainer>(
                                                 "raw_display", CHANNELS_PER_LINK * m_ids.size(), m_ids, 100, 0, 5000, false));
  }
  if (m_enable_mean_rms) {
    m_mean_rms.reset(std::make_shared<HistContainer>(
                                                     "rmsm_display", CHANNELS_PER_LINK * m_ids.size(), m_ids, 100, 0, 5000, true));
  }
  if (m_enable_fourier) {
    m_fourier.reest(std::make_shared<FourierContainer>("fft_display",
                                                    CHANNELS_PER_LINK * m_ids.size(),
                                                    m_ids,
                                                    1. / m_clock_frequency * TICKS_BETWEEN_TIMESTAMP,
                                                       m_standard_dqm_fourier.num_frames));
  }
  if (m_enable_fourier_sum) {
    m_fourier_sum.reset(std::make_shared<FourierContainer>("fft_sums_display",
                                                       4,
                                                       m_ids,
                                                       1. / m_clock_frequency * TICKS_BETWEEN_TIMESTAMP,
                                                       m_standard_dqm_fourier_sum.num_frames,
                                                           true));
  }
}

void
DFModule::run(std::unique_ptr<daqdataformats::TriggerRecord> record,
                   std::atomic<bool>& run_mark,
                   std::unique_ptr<SortedChannelMap>& map,
                   std::string kafka_address)
{
  set_is_running(true);
  if (m_enable_hist) {
    m_hist->run(record, run_mark, map, kafka_address);
  }
  if (m_enable_mean_rms) {
    m_mean_rms->run(record, run_mark, map, kafka_address);
  }
  if (m_enable_fourier) {
    m_fourier->run(record, run_mark, map, kafka_address);
  }
  if (m_enable_fouriersum) {
    m_fourier->run(record, run_mark, map, kafka_address);
  }
  set_is_running(false);
}


} // namespace dunedaq::dqm

#endif // DQM_SRC_DFMODULE_HPP_
