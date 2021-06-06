/**
 * @file PrintClass.hpp PrintClass Class Interface
 *
 * PrintClass is a class that illustrates how to make a DUNE DAQ
 * module that interacts with another one. It does some printing to the screen
 * and prints the trigger number of a Fragment
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/DAQModule.hpp"
#include <atomic>

namespace dunedaq::dqm {

class PrintClass : public dunedaq::appfwk::DAQModule
{

public:
 /**
 * @brief PrintClass Constructor
 * @param name Instance name for this PrintClass instance
 */
  explicit PrintClass(const std::string& name);

  PrintClass(const PrintClass&) =
    delete; ///< FakeCardReader is not copy-constructible
  PrintClass& operator=(const PrintClass&) =
    delete; ///< FakeCardReader is not copy-assignable
  PrintClass(PrintClass&&) =
    delete; ///< FakeCardReader is not move-constructible
  PrintClass& operator=(PrintClass&&) =
    delete; ///< FakeCardReader is not move-assignable

  void init(const data_t& ) override;

  void do_print(const data_t&);
  void do_start(const data_t&);
  void do_stop(const data_t&);

  std::atomic<bool> m_run_marker;

};


} // namespace dunedaq::dqm


