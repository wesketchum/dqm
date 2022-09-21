/**
 * @file FormatUtils.hpp Utility functions for different data formats
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_DQM_FORMATS_HPP_
#define DQM_INCLUDE_DQM_DQM_FORMATS_HPP_

#include <memory>
#include <atomic>

namespace dunedaq::dqm {

struct DQMArgs {
  std::shared_ptr<std::atomic<bool>> run_mark;
  std::shared_ptr<ChannelMap> map;
  std::string frontend_type;
  std::string kafka_address;
  std::string kafka_topic;
  int max_frames;
};

} // namespace dunedaq::dqm

#endif // DQM_INCLUDE_DQM_DQM_FORMATS_HPP_
