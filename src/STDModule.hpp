/**
 * @file STDModule.hpp Implementation of a container of Hist objects
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_STDMODULE_HPP_
#define DQM_SRC_STDMODULE_HPP_

// DQM
#include "dqm/algs/STD.hpp"
#include "ChannelStream.hpp"

#include <string>
#include <vector>

namespace dunedaq::dqm {

class STDModule : public ChannelStream<STD, double>
{

public:
  STDModule(std::string name,
            int nchannels,
            std::vector<int>& link_idx);
};

STDModule::STDModule(std::string name,
                             int nchannels,
                             std::vector<int>& link_idx
                     )
  : ChannelStream(name, nchannels, link_idx,
                  [this] (std::vector<STD>& vec, int ch, int link) -> std::vector<double> {
                    return {vec[get_local_index(ch, link)].std()};})
{
}

} // namespace dunedaq::dqm

#endif // DQM_SRC_STDMODULE_HPP_
