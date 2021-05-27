cmake_minimum_required(VERSION 3.12)
project(DQM VERSION 1.1.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

# Here we put what packages we need

find_package(appfwk REQUIRED)     
find_package(dataformats REQUIRED)
# find_package(dfmodules REQUIRED)
# find_package(ers REQUIRED)     
# find_package(TRACE REQUIRED)
# find_package(Boost COMPONENTS unit_test_framework program_options REQUIRED)

# We put here the name of our class, and the libraries we want to link with

daq_add_library()

daq_add_plugin(PrintClass duneDAQModule LINK_LIBRARIES appfwk::appfwk dataformats::dataformats DQM readout)
daq_add_plugin(DataReceiver duneDAQModule LINK_LIBRARIES appfwk::appfwk dataformats::dataformats DQM readout)

daq_install()



