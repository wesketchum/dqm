/**
 * @file PythonModule.hpp Implementation of a container of Fourier objects
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_DQM_MODULES_PYTHON_HPP_
#define DQM_INCLUDE_DQM_MODULES_PYTHON_HPP_

// DQM
#include "dqm/AnalysisModule.hpp"
#include "dqm/ChannelMap.hpp"
#include "dqm/Constants.hpp"
#include "dqm/Decoder.hpp"
#include "dqm/Issues.hpp"
#include "dqm/DQMFormats.hpp"
#include "dqm/DQMLogging.hpp"
#include "dqm/PythonUtils.hpp"
#include "dqm/Pipeline.hpp"

#include "daqdataformats/TriggerRecord.hpp"

#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <string>

#include <Python.h>
#include <boost/python.hpp>
#include <boost/python/errors.hpp>
#include <boost/python/numpy.hpp>

namespace dunedaq::dqm {

namespace p = boost::python;
namespace np = boost::python::numpy;

using logging::TLVL_WORK_STEPS;

class PythonModule : public AnalysisModule
{
  std::string m_name;

public:
  PythonModule(std::string name);

  void run(std::shared_ptr<daqdataformats::TriggerRecord> record,
      DQMArgs& args, DQMInfo& info) override;

  template <class T>
  void run_(std::shared_ptr<daqdataformats::TriggerRecord> record,
       DQMArgs& args, DQMInfo& info);

  void transmit_global(const std::string &kafka_address,
                       std::shared_ptr<ChannelMap> cmap,
                       const std::string& topicname,
                       int run_num);
};

PythonModule::PythonModule(std::string name)
  : m_name(std::string(name))
{
}

template <class T>
void
PythonModule::run_(std::shared_ptr<daqdataformats::TriggerRecord> record,
                       DQMArgs& args, DQMInfo& info)
{
  auto start = std::chrono::steady_clock::now();
  auto map = args.map;
  auto frames = decode<T>(record, args.max_frames);
  auto pipe = Pipeline<T>({"remove_empty", "check_empty", "make_same_size", "check_timestamps_aligned"});
  bool valid_data = pipe(frames);
  if (!valid_data) {
    return;
  }

  const char* module_name = (std::string("dqm.dqm_") + m_name).c_str();
  const char* partition = getenv("DUNEDAQ_PARTITION");
  const char* app_name = getenv("DUNEDAQ_APPLICATION_NAME");
  // TLOG() << "module_name is " << module_name;

  PyGILState_STATE gilState;
  gilState = PyGILState_Ensure();

  p::object module;
  try {
    module = p::import(module_name);
  } catch (p::error_already_set& e) {
    ers::error(ModuleNotImported(ERS_HERE, module_name));
    return;
  }

  typedef std::map<int, std::vector<T*>> mapt;
  auto channels = MapItem<mapt>::get_channels(frames, map);
  auto planes = MapItem<mapt>::get_planes(frames, map);

  p::object f = module.attr("main");
  f(frames, channels, planes,
    record->get_header_ref().get_run_number(),
    partition,
    app_name
    );
  PyGILState_Release(gilState);
  // try {
  //   f(frames, channels, planes);
  // } catch (...) {
  //   ers::error(PythonModuleCrashed(ERS_HERE, module_name));
  //   return;
  // }

  // info.fourier_plane_time_taken.store(std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count());
  // info.fourier_plane_times_run++;

}

void
PythonModule::run(std::shared_ptr<daqdataformats::TriggerRecord> record,
                      DQMArgs& args, DQMInfo& info)
{
  TLOG(TLVL_WORK_STEPS) << "Running "<< m_name << " with frontend_type = " << args.frontend_type;
  auto frontend_type = args.frontend_type;
  auto run_mark = args.run_mark;
  auto map = args.map;
  auto kafka_address = args.kafka_address;
  if (frontend_type == "wib") {
    set_is_running(true);
    run_<detdataformats::wib::WIBFrame>(std::move(record), args, info);
    set_is_running(false);
  }
  else if (frontend_type == "wib2") {
    set_is_running(true);
    run_<detdataformats::wib2::WIB2Frame>(std::move(record), args, info);
    set_is_running(false);
  }
}

} // namespace dunedaq::dqm

#endif // DQM_INCLUDE_DQM_MODULES_PYTHON_HPP_
