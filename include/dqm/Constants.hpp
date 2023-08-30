/**
 * @file Constants.hpp Define some constants used by DQM
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_CONSTANTS_HPP_
#define DQM_SRC_CONSTANTS_HPP_

#include <string>

namespace dunedaq::dqm {

constexpr int CHANNELS_PER_LINK = 256;

int
get_ticks_between_timestamps(const std::string& frontend_type)
{
  if (frontend_type == "wib") {
    return 25;
  }
  else if (frontend_type == "wib2") {
    return 32;
  }
  else if (frontend_type == "wibeth") {
    return 32*64;
  }
  else if (frontend_type == "daphnestream") {
    return 64;
  }
  return -1;
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_CONSTANTS_HPP_
