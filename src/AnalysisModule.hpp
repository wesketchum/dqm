#pragma once

/**
 * @file AnalysisModule.hpp Definition of the abstract class AnalysisModule
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include <atomic>

#include "dataformats/TriggerRecord.hpp"

namespace dunedaq::DQM {

class AnalysisModule {
public:
  virtual bool is_running() = 0;
  virtual void run(dataformats::TriggerRecord &record) = 0;
};

} //namespace dunedaq::DQM
