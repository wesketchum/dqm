/**
 * @file StDev_test.cxx Unit Tests for StDev calculations
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

/**
 * @brief Name of this test module
 */
#define BOOST_TEST_MODULE StDev_test // NOLINT

#include "boost/test/unit_test.hpp"

#include "dqm/algs/STD.hpp"

#include <algorithm>
#include <random>
#include <vector>

using namespace dunedaq::dqm;

BOOST_AUTO_TEST_SUITE(StDev_test)

std::mt19937 mt(1000007);

void
StDev_test_case(int n, double low, double high)
{

  std::uniform_real_distribution<double> dist(low, high);

  STD std;
  std::vector<double> v;
  for (int i = 0; i < n; i++) {
    double num = dist(mt);
    std.fill(num);
    v.push_back(num);
  }
  double mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
  std::for_each(v.begin(), v.end(), [&mean](double& x) { x = (x - mean) * (x - mean); });
  double res = sqrt(std::accumulate(v.begin(), v.end(), 0.0) / (v.size() - 1));

  std::cout << std.std() << " " << res << std::endl;
  BOOST_TEST_REQUIRE(abs(std.std() - res) < 1e-4);
}

BOOST_AUTO_TEST_CASE(StDev_test1)
{
  StDev_test_case(200, 0, 1);
}

BOOST_AUTO_TEST_CASE(StDev_test2)
{
  StDev_test_case(599, -4, 8);
}

BOOST_AUTO_TEST_CASE(StDev_test3)
{
  StDev_test_case(1000, 10000, 20000);
}

BOOST_AUTO_TEST_CASE(StDev_test4)
{
  StDev_test_case(30000, 0, 20000);
}

BOOST_AUTO_TEST_CASE(StDev_test5)
{
  StDev_test_case(100000, 0, 100000);
}

BOOST_AUTO_TEST_SUITE_END()
