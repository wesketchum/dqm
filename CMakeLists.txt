cmake_minimum_required(VERSION 3.12)
project(dqm VERSION 1.6.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

# Here we put what packages we need

find_package(appfwk REQUIRED)
find_package(daqdataformats REQUIRED)
find_package(detdataformats REQUIRED)
find_package(fddetdataformats REQUIRED)
find_package(detchannelmaps REQUIRED)
find_package(dfmessages REQUIRED)
find_package(iomanager REQUIRED)
find_package(RdKafka REQUIRED)
find_package(timinglibs REQUIRED)
find_package(Boost COMPONENTS unit_test_framework program_options REQUIRED)
find_package(fftw REQUIRED)

daq_codegen(*.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )
daq_codegen(*info.jsonnet DEP_PKGS opmonlib TEMPLATES opmonlib/InfoStructs.hpp.j2 opmonlib/InfoNljs.hpp.j2 )

# The value is stored in CMakeCache.txt so if you change the option make sure to remove the file or build cleanly
option(WITH_PYTHON_SUPPORT "Boost Python is available and algorithms defined in Python can be used" OFF)

set(DQM_DEPENDENCIES
appfwk::appfwk
detdataformats::detdataformats
fddetdataformats::fddetdataformats
detchannelmaps::detchannelmaps
dfmessages::dfmessages
timinglibs::timinglibs
)

if(WITH_PYTHON_SUPPORT)
  string(REGEX MATCH "/[^:]+python[^:]+" PYTHON_LIB_PATH $ENV{LD_LIBRARY_PATH})
  add_compile_definitions(WITH_PYTHON_SUPPORT)
  set(DQM_DEPENDENCIES ${DQM_DEPENDENCIES}
    ${PYTHON_LIB_PATH}/libpython3.10.so
    $ENV{BOOST_PYTHON}/libboost_python310.so
    $ENV{BOOST_PYTHON}/libboost_numpy310.so
)
else()
  set(DQM_DEPENDENCIES ${DQM_DEPENDENCIES}
  RdKafka::rdkafka
  RdKafka::rdkafka++
  fftw::fftw3
  )
  daq_add_library(
    algs/*.cpp
    LINK_LIBRARIES ${DQM_DEPENDENCIES}
  )
  set(DQM_DEPENDENCIES ${DQM_DEPENDENCIES} dqm)
endif()

daq_add_plugin(DQMProcessor duneDAQModule LINK_LIBRARIES ${DQM_DEPENDENCIES})

if(WITH_PYTHON_SUPPORT)
  daq_add_application(python_test python_test.cxx TEST LINK_LIBRARIES ${DQM_DEPENDENCIES})
else()
  target_include_directories(dqm_DQMProcessor_duneDAQModule PUBLIC $ENV{FFTW_INC} )
  # Unit tests (C++)
  daq_add_unit_test(Fourier_test LINK_LIBRARIES ${DQM_DEPENDENCIES})
  daq_add_unit_test(RMS_test LINK_LIBRARIES ${DQM_DEPENDENCIES})
  daq_add_unit_test(STD_test LINK_LIBRARIES ${DQM_DEPENDENCIES})
endif()

daq_install()
