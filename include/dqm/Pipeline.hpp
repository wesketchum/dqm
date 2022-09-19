/**
 * @file Decoder.hpp Implementation of decoders used in DQM analysis modules
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_PIPELINE_HPP_
#define DQM_SRC_PIPELINE_HPP_

#include "ers/Issue.hpp"
#include "dqm/Issues.hpp"

#include <cstddef>
#include <map>
#include <vector>
#include <functional>
#include <string>

namespace dunedaq {
namespace dqm {

template<class T>
int
remove_empty(std::map<int, std::vector<T*>>& map)
{
  bool empty_fragments = false;
  for (auto& [key, val] : map)
    if (not val.size()) {
      empty_fragments = true;
      map.erase(key);
    }
  if (empty_fragments) {
    ers::warning(EmptyFragments(ERS_HERE, ""));
  }
  return true;
}

template<class T>
int
check_empty(std::map<int, std::vector<T*>>& map)
{
  if (not map.size()) {
    ers::error(EmptyData(ERS_HERE, ""));
    return false;
  }
  return true;
}

template<class T>
int
make_same_size(std::map<int, std::vector<T*>>& map)
{
  std::vector<size_t> sizes;
  std::map<size_t, int> occurrences;
  for (auto& [key, val] : map) {
    occurrences[val.size()]++;
  }
  if (occurrences.size() == 1) {
    return true;
  }

  ers::warning(TimestampsNotAligned(ERS_HERE, ""));

  return true;
}

template<class T>
int
check_timestamps_aligned(std::map<int, std::vector<T*>>& map)
{
  return true;
}


template<class T>
class Pipeline
{
  std::vector<std::function<int(std::map<int, std::vector<T*>>& arg)>> m_functions;
  std::vector<std::string> m_function_names;

  std::map<std::string, std::function<int(std::map<int, std::vector<T*>>& arg)>>
  m_available_functions { {"remove_empty", remove_empty<T>},
                          {"check_empty", check_empty<T>},
                          {"make_same_size", make_same_size<T>},
                          {"check_timestamp_aligned", check_timestamps_aligned<T>},
  };

public:
  Pipeline(std::vector<std::string>& names) {
    for (auto& name: names) {
      if (m_available_functions.find(name) != m_available_functions.end()) {
        m_function_names.push_back(name);
        m_functions.push_back(m_available_functions[name]);
      }
    }
  }
  Pipeline(std::vector<std::string>&& names) {
    for (auto& name: names) {
      if (m_available_functions.find(name) != m_available_functions.end()) {
        m_function_names.push_back(name);
        m_functions.push_back(m_available_functions[name]);
      }
    }

  }

  void operator() (std::map<int, std::vector<T*>>& arg) {
    for (size_t i = 0; i < m_functions.size(); ++i) {
      auto fun = m_functions[i];
      auto name = m_function_names[i];
      int ret = fun(arg);
      if (not ret) {
        TLOG() << "Unable to continue";
        return;
      }
      else {
      }
    }
  }

};


} // namespace dqm
} // namespace dunedaq

#endif // DQM_SRC_PIPELINE_HPP_
