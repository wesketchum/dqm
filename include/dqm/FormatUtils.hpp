/**
 * @file FormatUtils.hpp Utility functions for different data formats
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_FORMATUTILS_HPP_
#define DQM_INCLUDE_DQM_FORMATUTILS_HPP_

#include "detdataformats/wib/WIBFrame.hpp"
#include "detdataformats/wib2/WIB2Frame.hpp"

namespace dunedaq::dqm {

template<class T>
inline int get_crate(T* fr) {
  return -1;
}

template <>
inline int get_crate(detdataformats::wib::WIBFrame* fr) {
  return fr->get_wib_header()->crate_no;
}

template <>
inline int get_crate(detdataformats::wib2::WIB2Frame* fr) {
  return fr->header.crate;
}

template<class T>
inline int get_slot(T* fr) {
  return -1;
}

template <>
inline int get_slot(detdataformats::wib::WIBFrame* fr) {
  return fr->get_wib_header()->slot_no;
}

template <>
inline int get_slot(detdataformats::wib2::WIB2Frame* fr) {
  return fr->header.slot;
}

template<class T>
inline int get_fiber(T* fr) {
  return -1;
}

template <>
inline int get_fiber(detdataformats::wib::WIBFrame* fr) {
  return fr->get_wib_header()->fiber_no;
}

template <>
inline int get_fiber(detdataformats::wib2::WIB2Frame* fr) {
  return fr->header.link;
}

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

#endif // DQM_INCLUDE_DQM_FORMATUTILS_HPP_
