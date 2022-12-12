/**
 * @file Decoder.hpp Implementation of decoders used in DQM analysis modules
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_PIPELINE_HPP_
#define DQM_SRC_PIPELINE_HPP_

#include "DQMLogging.hpp"
#include "dqm/DQMFormats.hpp"
#include "dqm/DQMLogging.hpp"
#include "dqm/Issues.hpp"
#include "ers/Issue.hpp"

#include <cstddef>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace dunedaq {
namespace dqm {

using logging::TLVL_WORK_STEPS;

template<class T>
int
remove_empty(std::map<int, std::vector<T*>>& map)
{
  TLOG(TLVL_WORK_STEPS) << "Removing empty fragments";
  bool empty_fragments = false;
  for (auto& [key, val] : map)
    if (not val.size() || val.size() == sizeof(daqdataformats::FragmentHeader)) {
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
  TLOG(TLVL_WORK_STEPS) << "Checking if the data is empty";
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
  TLOG(TLVL_WORK_STEPS) << "Making the fragments have the same size";
  std::vector<size_t> sizes;
  std::map<size_t, int> occurrences;
  for (const auto& [key, val] : map) {
    occurrences[val.size()]++;
  }
  if (occurrences.size() == 1) {
    return true;
  }
  auto min_size = (*occurrences.begin()).first;
  for (auto& [key, vec] : map) {
    vec.resize(min_size);
  }

  return true;
}

template<class T>
int
check_timestamps_aligned(std::map<int, std::vector<T*>>& map)
{
  TLOG(TLVL_WORK_STEPS) << "Checking that the timestamps are aligned";
  uint64_t timestamp = get_timestamp<T>((*map.begin()).second[0]);
  bool different_timestamp = false;
  for (const auto& [key, val] : map) {
    if (get_timestamp<T>(*val.begin()) != timestamp) {
      different_timestamp = true;
      break;
    }
  }
  if (different_timestamp) {
    ers::warning(TimestampsNotAligned(ERS_HERE, ""));
  }
  return true;
}

template<class T>
class Pipeline
{
  std::vector<std::function<int(std::map<int, std::vector<T*>>& arg)>> m_functions;
  std::vector<std::string> m_function_names;

  std::map<std::string, std::function<int(std::map<int, std::vector<T*>>& arg)>> m_available_functions{
    { "remove_empty", remove_empty<T> },
    { "check_empty", check_empty<T> },
    { "make_same_size", make_same_size<T> },
    { "check_timestamps_aligned", check_timestamps_aligned<T> },
  };

public:
  Pipeline(std::vector<std::string>& names)
  {
    for (auto& name : names) {
      if (m_available_functions.find(name) != m_available_functions.end()) {
        m_function_names.push_back(name);
        m_functions.push_back(m_available_functions[name]);
      }
    }
  }
  Pipeline(std::vector<std::string>&& names)
  {
    for (auto& name : names) {
      if (m_available_functions.find(name) != m_available_functions.end()) {
        m_function_names.push_back(name);
        m_functions.push_back(m_available_functions[name]);
      }
    }
  }

  bool operator()(std::map<int, std::vector<T*>>& arg)
  {
    for (size_t i = 0; i < m_functions.size(); ++i) {
      auto fun = m_functions[i];
      auto name = m_function_names[i];
      int ret = fun(arg);
      if (!ret) {
        return false;
      }
    }
    return true;
  }
};

} // namespace dqm
} // namespace dunedaq

#endif // DQM_SRC_PIPELINE_HPP_
