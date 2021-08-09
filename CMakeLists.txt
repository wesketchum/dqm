cmake_minimum_required(VERSION 3.12)
project(dqm VERSION 1.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

daq_codegen(*.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )

# Here we put what packages we need

find_package(readout REQUIRED)
find_package(appfwk REQUIRED)     
find_package(dataformats REQUIRED)
find_package(timinglibs REQUIRED)
find_package(RdKafka REQUIRED)

# We put here the name of our class, and the libraries we want to link with

daq_add_plugin(DQMProcessor duneDAQModule LINK_LIBRARIES appfwk::appfwk dataformats::dataformats readout::readout timinglibs::timinglibs RdKafka::rdkafka RdKafka::rdkafka++)

daq_install()
