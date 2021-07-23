/**
 * @file dfmessages/Types.hpp Type definitions used in DQM
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_TYPES_HPP_
#define DQM_INCLUDE_DQM_TYPES_HPP_

namespace dunedaq::dqm {

/**
 * @brief DQM can be run in debug and normal modes
 */
enum class RunningMode
{
  kLocalProcessing,
  kNormal
};

} // namespace dunedaq::dqm

#endif // DQM_INCLUDE_DQM_TYPES_HPP_
