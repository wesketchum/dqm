
/**
 * @file CounterModule.hpp Implementation of the module for the raw data stream
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_COUNTERMODULE_HPP_
#define DQM_SRC_COUNTERMODULE_HPP_

// DQM
#include "dqm/AnalysisModule.hpp"
#include "dqm/ChannelMap.hpp"
#include "dqm/Constants.hpp"
#include "dqm/Decoder.hpp"
#include "dqm/Exporter.hpp"
#include "dqm/Issues.hpp"
#include "dqm/algs/Counter.hpp"
#include "dqm/FormatUtils.hpp"
#include "dqm/Pipeline.hpp"
#include "dqm/DQMLogging.hpp"
#include "dqm/ChannelStream.hpp"

#include "daqdataformats/TriggerRecord.hpp"
#include "detdataformats/tde/TDE16Frame.hpp"

#include <cstdlib>
#include <map>
#include <string>
#include <vector>

namespace dunedaq::dqm {

class CounterModule : public ChannelStream<Counter, int>
{

public:
  CounterModule(std::string name,
            int nchannels,
            std::vector<int>& link_idx);

};

CounterModule::CounterModule(std::string name,
                             int nchannels,
                             std::vector<int>& link_idx
                     )
  : ChannelStream(name, nchannels, link_idx,
                  [this] (std::vector<Counter>& vec, int ch, int link) -> std::vector<int> {
                    return vec[get_local_index(ch, link)].count;})
{
}


} // namespace dunedaq::dqm

#endif // DQM_SRC_COUNTERMODULE_HPP_
