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
#include "readout/ReadoutTypes.hpp"
#include "Hist.hpp"
#include "Fourier.hpp"
#include "Exporter.hpp"

#include <thread>


namespace dunedaq{
namespace DQM {

PrintClass::PrintClass(const std::string& name) : DAQModule(name)
{
  std::cout << name << std::endl;
  register_command("start", &PrintClass::do_start);
  register_command("stop", &PrintClass::do_stop);
}

void
PrintClass::init(const data_t& args)
{
  std::cout << "Printing init!!!" << std::endl;
}

void
PrintClass::do_start(const data_t&)
{
  m_run_marker.store(true);

  appfwk::DAQSource< std::unique_ptr<dataformats::Fragment >> source("data_fragments_q");
  std::unique_ptr<dataformats::Fragment, std::default_delete<dataformats::Fragment> > element;
  int i = 0;
  Hist hist(100, 0, 2000);
  while(true) {
    source.pop(element, std::chrono::milliseconds(1000));
    std::cout << "!!!!!!!!!!!!!" << std::endl;
    std::cout << "One element" << std::endl;
    std::cout << "!!!!!!!!!!!!!" << std::endl;
    std::cout << element.get()->get_window_begin() << std::endl;
    auto data = static_cast<readout::types::WIB_SUPERCHUNK_STRUCT*>(element.get()->get_data());
    // for (int i=0; i<readout::types::WIB_SUPERCHUNK_SIZE; ++i)
    //     std::cout << (int)data->data[i] << " ";
    std::cout << std::endl;
    if (i <= 10){
      unsigned int num;
      for (int i=0; i<readout::types::WIB_SUPERCHUNK_SIZE / sizeof(num); ++i){
        memcpy(&num, data->data + i * sizeof(num), sizeof(num));
        // num = __builtin_bswap32(num);
        hist.fill(num);
        std::cout << num << " ";
      }
      std::cout << std::endl;
    }
    if (i == 10)
    hist.save("histogram.txt");
    ++i; 
  }

}

void
PrintClass::do_stop(const data_t&)
{
  m_run_marker.store(false);

}

} // namespace dunedaq
} // namespace DQM

// Needed to define a module
DEFINE_DUNE_DAQ_MODULE(dunedaq::DQM::PrintClass)
