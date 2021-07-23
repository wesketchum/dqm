/**
 * @file AnalysisModule.hpp Definition of the abstract class AnalysisModule
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_ANALYSISMODULE_HPP_
#define DQM_SRC_ANALYSISMODULE_HPP_

#include "dqm/Types.hpp"

#include "dataformats/TriggerRecord.hpp"

#include <atomic>

namespace dunedaq::dqm {

class AnalysisModule
{
public:
  virtual bool is_running() = 0;
  virtual void run(dataformats::TriggerRecord& record, RunningMode mode) = 0;
};

} // namespace dunedaq::dqm

#endif // DQM_SRC_ANALYSISMODULE_HPP_
