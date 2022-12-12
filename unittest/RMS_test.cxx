/**
 * @file RMS_test.cxx Unit Tests for RMS calculations
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

/**
 * @brief Name of this test module
 */
#define BOOST_TEST_MODULE RMS_test // NOLINT

#include "boost/test/unit_test.hpp"

#include "dqm/algs/RMS.hpp"

#include <numeric>
#include <random>
#include <vector>

using namespace dunedaq::dqm;

BOOST_AUTO_TEST_SUITE(RMS_test)

std::mt19937 mt(1000007);

void
RMS_test_case(int n, double low, double high)
{

  std::uniform_real_distribution<double> dist(low, high);

  RMS rms;
  std::vector<double> v;
  for (int i = 0; i < n; i++) {
    double num = dist(mt);
    rms.fill(num);
    v.push_back(num);
  }
  double res = sqrt(std::inner_product(v.begin(), v.end(), v.begin(), 0.0) / v.size());

  BOOST_TEST_REQUIRE(abs(rms.rms() - res) < 1e-4);
}

BOOST_AUTO_TEST_CASE(RMS_test1)
{
  RMS_test_case(200, 0, 1);
}

BOOST_AUTO_TEST_CASE(RMS_test2)
{
  RMS_test_case(599, -4, 8);
}

BOOST_AUTO_TEST_CASE(RMS_test3)
{
  RMS_test_case(1000, 10000, 20000);
}

BOOST_AUTO_TEST_CASE(RMS_test4)
{
  RMS_test_case(30000, 0, 20000);
}

BOOST_AUTO_TEST_CASE(RMS_test5)
{
  RMS_test_case(100000, 0, 100000);
}

BOOST_AUTO_TEST_SUITE_END()
