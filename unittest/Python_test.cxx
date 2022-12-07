/**
 * @file Python_test.cxx Unit Tests for python and boost/python
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

/**
 * @brief Name of this test module
 */
#define BOOST_TEST_MODULE Python_test // NOLINT

#include "boost/test/unit_test.hpp"

#include "boost/python.hpp"

#include <vector>
#include <random>
#include <algorithm>

// using namespace dunedaq::dqm;

BOOST_AUTO_TEST_SUITE(StDev_test)

std::mt19937 mt(1000007);

void
StDev_test_case(int n, double low, double high)
{
  try {
    module = p::import("dqm.dqm_std");
  } catch (p::error_already_set& e) {
    ers::error(ModuleNotImported(ERS_HERE, module_name));
    return;
  }
  BOOST_TEST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(StDev_test1)
{
  StDev_test_case(200, 0, 1);
}


BOOST_AUTO_TEST_SUITE_END()
