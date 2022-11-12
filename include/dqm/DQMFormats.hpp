/**
 * @file FormatUtils.hpp Utility functions for different data formats
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_DQM_FORMATS_HPP_
#define DQM_INCLUDE_DQM_DQM_FORMATS_HPP_

#include "dqm/ChannelMap.hpp"

#include <memory>
#include <map>
#include <string>

namespace dunedaq::dqm {

struct DQMArgs {
  std::shared_ptr<std::atomic<bool>> run_mark;
  std::shared_ptr<ChannelMap> map;
  std::string frontend_type;
  std::string kafka_address;
  std::string kafka_topic;
  int max_frames;
};

struct DQMInfo {
  std::atomic<int> channel_map_total_channels,
                   channel_map_total_planes,
                   raw_times_run,
                   std_times_run,
                   rms_times_run,
                   fourier_channel_times_run,
                   fourier_plane_times_run;

  std::atomic<float> raw_time_taken,
                     std_time_taken,
                     rms_time_taken,
                     fourier_channel_time_taken,
                     fourier_plane_time_taken;

};

} // namespace dunedaq::dqm

#endif // DQM_INCLUDE_DQM_DQM_FORMATS_HPP_
