/**
 * @file FormatUtils.hpp Utility functions for different data formats
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_FRAMEUTILS_HPP_
#define DQM_SRC_FRAMEUTILS_HPP_

#include "detdataformats/wib/WIBFrame.hpp"
#include "detdataformats/wib2/WIB2Frame.hpp"

namespace dunedaq::dqm {

template <class T>
inline uint64_t get_timestamp(T* fr) {
  return fr->get_timestamp();
}

template <class T>
inline uint16_t get_adc(T* fr, int ch) {
  return -1;
}

template <>
inline uint16_t get_adc(detdataformats::wib::WIBFrame* fr, int ch) {
  return fr->get_channel(ch);
}

template <>
inline uint16_t get_adc(detdataformats::wib2::WIB2Frame* fr, int ch) {
  return fr->get_adc(ch);
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_FRAMEUTILS_HPP_
