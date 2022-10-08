/**
 * @file RMSModule.hpp Implementation of a container of Hist objects
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_RMSMODULE_HPP_
#define DQM_SRC_RMSMODULE_HPP_

// DQM
#include "dqm/algs/RMS.hpp"
#include "ChannelStream.hpp"

#include <string>
#include <vector>

namespace dunedaq::dqm {

class RMSModule : public ChannelStream<RMS>
{

public:
  RMSModule(std::string name,
            int nchannels,
            std::vector<int>& link_idx);

};


RMSModule::RMSModule(std::string name,
                             int nchannels,
                             std::vector<int>& link_idx
                     )
  : ChannelStream(name, nchannels, link_idx,
                  [this] (std::vector<RMS>& vec, int ch, int link) {
                    return vec[get_local_index(ch, link)].rms();})
{
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_RMSMODULE_HPP_
