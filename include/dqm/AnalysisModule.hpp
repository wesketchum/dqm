/**
 * @file AnalysisModule.hpp Definition of the abstract class AnalysisModule
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_ANALYSISMODULE_HPP_
#define DQM_SRC_ANALYSISMODULE_HPP_

#include "ChannelMap.hpp"
#include "DQMFormats.hpp"

#include "daqdataformats/TriggerRecord.hpp"

#include <atomic>
#include <memory>
#include <string>

namespace dunedaq::dqm {

class AnalysisModule
{
public:

  bool get_is_running() const { return m_is_running; }

  virtual void run(std::shared_ptr<daqdataformats::TriggerRecord> record,
                                                             DQMArgs& args,
                                                             DQMInfo& info
                                                             ) = 0;
  virtual ~AnalysisModule() {};

protected:
  void set_is_running(bool status) { m_is_running = status; }

private:
  std::atomic<bool> m_is_running = false;
};

} // namespace dunedaq::dqm

#endif // DQM_SRC_ANALYSISMODULE_HPP_
