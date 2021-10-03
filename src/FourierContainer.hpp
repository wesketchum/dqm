/**
 * @file FourierContainer.hpp Implementation of a container of Fourier objects
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_SRC_FOURIERCONTAINER_HPP_
#define DQM_SRC_FOURIERCONTAINER_HPP_

// DQM
#include "AnalysisModule.hpp"
#include "Decoder.hpp"
#include "Exporter.hpp"
#include "dqm/Fourier.hpp"
#include "dqm/Types.hpp"

#include "dataformats/TriggerRecord.hpp"

#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

namespace dunedaq::dqm {

class FourierContainer : public AnalysisModule
{
  std::string m_name;
  std::vector<Fourier> fouriervec;
  size_t m_size;

public:
  FourierContainer(std::string name, int size, double inc, int npoints);

  void run(dunedaq::dataformats::TriggerRecord& tr, RunningMode mode = RunningMode::kNormal, std::string kafka_address="");
  void transmit(std::string &kafka_address, const std::string& topicname, int run_num, time_t timestamp);
  // void clean();
  // void save_and_clean(uint64_t timestamp); // NOLINT(build/unsigned)

};

  FourierContainer::FourierContainer(std::string name, int sizedouble, double inc, int npoints)
  : m_name(name)
  , m_size(npoints)
{
  for (size_t i = 0; i < m_size; ++i)
    fouriervec.emplace_back(Fourier(inc, npoints));
}

void
FourierContainer::run(dunedaq::dataformats::TriggerRecord& tr, RunningMode, std::string kafka_address)
{
  dunedaq::dqm::Decoder dec;
  auto wibframes = dec.decode(tr);
  // std::uint64_t timestamp = 0; // NOLINT(build/unsigned)
  m_run_mark.store(true);

  for (auto fr : wibframes) {

    for (size_t ich = 0; ich < m_size; ++ich)
      fouriervec[ich].fill(fr->get_channel(ich));

    // Debug mode - save to a file
    // if (mode != RunningMode::kLocalProcessing) continue;
    // if (++frames_run == max_frames) // NOLINT(runtime/increment_decrement)
    // {
    //   timestamp = fr->get_wib_header()->get_timestamp();
    //   if (mode == RunningMode::kLocalProcessing) {save_and_clean(timestamp);
    //   } else if (mode == RunningMode::kNormal) {
    //     transmit(kafka_address, "testdunedqm", tr.get_header_ref().get_run_number(), tr.get_header_ref().get_trigger_timestamp());
    //   }
    //   frames_run = 0;
    //   ++current_index;
    //   if (current_index == 2000) {
    //     ++filename_index;
    //     current_index = 0;
    //   }
    // }
  }
  // for (size_t ich = 0; ich < m_size; ++ich)
  //   auto out = fouriervec[ich].compute_fourier_def();

  // Transmit via kafka
  // if (mode == RunningMode::kNormal){
  //
  // }

  m_run_mark.store(false);

  // clean();
}

void
FourierContainer::transmit(std::string& kafka_address, const std::string& topicname, int run_num, time_t timestamp)
{
  std::stringstream csv_output;
  std::string datasource = "TESTSOURCE";
  std::string dataname = this->m_name;
  std::string axislabel = "TESTLABEL";
  std::stringstream metadata;
  // metadata << fouriervec[0].m_steps << " " << fouriervec[0].m_low << " " << fouriervec[0].m_high;

  int subrun = 0;
  int event = 0;

  // Construct CSV output
  csv_output << datasource << ";" << dataname << ";" << run_num << ";" << subrun << ";" << event << ";" << timestamp
             << ";" << metadata.str() << ";";
  csv_output << axislabel << "\n";
  for (int ich = 0; ich < 256; ++ich) {
    csv_output << "Histogram_" << ich + 1 << "\n";
    // csv_output << fouriervec[ich].m_sum;
    csv_output << "\n";
  }
  // csv_output << "\n";

  // Transmit
  KafkaExport(kafka_address, csv_output.str(), topicname);

  // clean();
}

// void
// FourierContainer::clean()
// {
//   for (int ich = 0; ich < m_size; ++ich) {
//     fouriervec[ich].clean();
//   }
// }

// void
// FourierContainer::save_and_clean(uint64_t timestamp) // NOLINT(build/unsigned)
// {
//   std::ofstream file;
//   file.open("Hist/" + m_name + "-" + std::to_string(filename_index) + ".txt", std::ios_base::app);
//   file << timestamp << std::endl;

//   for (int ich = 0; ich < m_size; ++ich) {
//     file << fouriervec[ich].m_sum << " ";
//   }
//   file << std::endl;
//   file.close();

//   clean();
// }

} // namespace dunedaq::dqm

#endif // DQM_SRC_FOURIERCONTAINER_HPP_
