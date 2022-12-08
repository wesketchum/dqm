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

BOOST_AUTO_TEST_SUITE(Python_test)

void
Python_import_test_case()
{
  try {
    module = p::import("dqm.dqm_std");
  } catch (p::error_already_set& e) {
    ers::error(ModuleNotImported(ERS_HERE, module_name));
    return;
  }
  BOOST_TEST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(Python_test1)
{
  Python_import_test_case();
}


BOOST_AUTO_TEST_SUITE_END()
