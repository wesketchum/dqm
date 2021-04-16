/**
 * @file PrintClass.cpp PrintClass Class Implementation
 *
 * See header for more on this class
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "PrintClass.hpp"
#include "dataformats/Fragment.hpp"
#include "appfwk/DAQSource.hpp"

namespace dunedaq{
namespace DQM {

PrintClass::PrintClass(const std::string& name) : DAQModule(name)
{
  std::cout << name << std::endl;
  register_command("start", &PrintClass::pop);
}

void
PrintClass::init(const data_t& args)
{
  std::cout << "Printing init!!!" << std::endl;
}

void
PrintClass::pop(const data_t&)
{
  appfwk::DAQSource< std::unique_ptr<dataformats::Fragment, std::default_delete<dataformats::Fragment> >> source("data_fragments_q");
  std::unique_ptr<dataformats::Fragment, std::default_delete<dataformats::Fragment> > element;
  source.pop(element, std::chrono::milliseconds(5000));
  std::cout << "!!!!!!!!!!!!!" << std::endl;
  std::cout << "One element" << std::endl;
  std::cout << "!!!!!!!!!!!!!" << std::endl;
  std::cout << element.get()->get_trigger_number() << std::endl;

}

} // namespace dunedaq
} // namespace DQM

// Needed to define a module
DEFINE_DUNE_DAQ_MODULE(dunedaq::DQM::PrintClass)
