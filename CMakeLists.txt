cmake_minimum_required(VERSION 3.12)
project(dqm VERSION 1.5.2)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

# Here we put what packages we need

find_package(daqdataformats REQUIRED)
find_package(detdataformats REQUIRED)
find_package(detchannelmaps REQUIRED)
find_package(dfmessages REQUIRED)
find_package(iomanager REQUIRED)
find_package(RdKafka REQUIRED)
find_package(timinglibs REQUIRED)
find_package(Boost COMPONENTS unit_test_framework program_options REQUIRED)
find_package(fftw REQUIRED)

daq_codegen(*.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )
daq_codegen(*info.jsonnet DEP_PKGS opmonlib TEMPLATES opmonlib/InfoStructs.hpp.j2 opmonlib/InfoNljs.hpp.j2 )

# The vaule is stored in CMakeCache.txt so if you change the option make sure to remove the file or build cleanly
option(WITH_PYTHON_SUPPORT "Boost Python is available and algorithms defined in Python can be used" ON)
if(WITH_PYTHON_SUPPORT)
  add_compile_definitions(WITH_PYTHON_SUPPORT)
endif()

set(DQM_DEPENDENCIES
daqdataformats::daqdataformats
detdataformats::detdataformats
detchannelmaps::detchannelmaps
dfmessages::dfmessages
RdKafka::rdkafka
RdKafka::rdkafka++
timinglibs::timinglibs
fftw::fftw3
)

daq_add_library(
  algs/*.cpp
  LINK_LIBRARIES ${DQM_DEPENDENCIES}
)

daq_add_plugin(DQMProcessor duneDAQModule LINK_LIBRARIES ${DQM_DEPENDENCIES} dqm)
target_include_directories(dqm_DQMProcessor_duneDAQModule PUBLIC $ENV{FFTW_INC})

# Unit tests
daq_add_unit_test(Fourier_test LINK_LIBRARIES ${DQM_DEPENDENCIES} dqm)
daq_add_unit_test(RMS_test LINK_LIBRARIES ${DQM_DEPENDENCIES} dqm)
daq_add_unit_test(STD_test LINK_LIBRARIES ${DQM_DEPENDENCIES} dqm)

if(WITH_PYTHON_SUPPORT)
  daq_add_application(python_test python_test.cxx TEST LINK_LIBRARIES ${DQM_DEPENDENCIES} dqm /cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-v1.0/spack-0.18.1-gcc-12.1.0/spack-0.18.1/opt/spack/gcc-12.1.0/python-3.10.4-pvcs7oznmvimvho7rik5qugob7qlc4p2/lib/libpython3.10.so $ENV{BOOST_PYTHON}/libboost_python310.so $ENV{BOOST_PYTHON}/libboost_numpy310.so)
  target_include_directories(python_test PUBLIC /cvmfs/dunedaq.opensciencegrid.org/spack/externals/ext-v1.0/spack-0.18.1-gcc-12.1.0/spack-0.18.1/opt/spack/gcc-12.1.0/py-pybind11-2.6.2-huldi2a2krn3a44fwvqolsi6gr6m3pwa/include)
endif()

daq_install()
