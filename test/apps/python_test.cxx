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

#include "boost/python.hpp"

#include <vector>
#include <random>
#include <algorithm>
#include <map>
#include <memory>

#include <boost/python.hpp>
#include <boost/python/errors.hpp>
#include <boost/python/numpy.hpp>
// #include <pybind11/stl.h>

#include "fddetdataformats/WIBFrame.hpp"
#include "daqdataformats/TriggerRecord.hpp"
#include "daqdataformats/SourceID.hpp"
#include "dqm/FormatUtils.hpp"

using namespace boost::python;

#include <iostream>

namespace p = boost::python;
namespace np = boost::python::numpy;

using namespace dunedaq;

class MyClass {
    std::vector<int> contents;
};

/* ... binding code ... */

void IndexError() { PyErr_SetString(PyExc_IndexError, "Index out of range"); }
template<class T>
struct std_item
{
    typedef typename T::value_type V;
    static V& get(T& x, int i)
    {
        if( i<0 ) i+=x.size();
        if( i>=0 && i<x.size() ) return x[i];
        IndexError();
    }
    static void set(T const& x, int i, V const& v)
    {
        if( i<0 ) i+=x.size();
        if( i>=0 && i<x.size() ) x[i]=v;
        else IndexError();
    }
    static void del(T const& x, int i)
    {
        if( i<0 ) i+=x.size();
        if( i>=0 && i<x.size() ) x.erase(i);
        else IndexError();
    }
    static void add(T const& x, V const& v)
    {
        x.push_back(v);
    }
};
void KeyError() { PyErr_SetString(PyExc_KeyError, "Key not found"); }
template<class T>
class MapItem
{
public:
  // std::shared_ptr<std::map<int, std::vector<fddetdataformats::WIBFrame*>>> ptr;
  // MapItem (std::map<int, std::vector<fddetdataformats::WIBFrame*>> map_with_frames) {
  //   ptr = std::make_shared<std::map<int, std::vector<fddetdataformats::WIBFrame*>>>(map_with_frames);
  // }
  typedef typename T::key_type K;
  typedef typename T::mapped_type V;
  static np::ndarray get(T & x, K const& i)
  {
    std::cout << "Calling get" << std::endl;
    if( x.find(i) != x.end() ) {
      p::tuple shape = p::make_tuple(3, 3);
      np::dtype dtype = np::dtype::get_builtin<float>();
      np::ndarray a = np::zeros(shape, dtype);

      std::cout << "Returning!" << std::endl;
      // return x[i];
      return {a};
    }
      KeyError();
  }

  static std::vector<np::ndarray> get_adc(T & x)
  {
    np::dtype dtype = np::dtype::get_builtin<int>();

    std::vector<np::ndarray> v;
    for (const auto& [key, val] : x){
      std::cout << "key = " << key << std::endl;
      p::tuple shape = p::make_tuple(val.size(), 256);
      np::ndarray ary = np::empty(shape, dtype);
      int i = 0;
      for (const auto& fr : val) {
        for (int j = 0; j < 256; ++j) {
          ary[i][j] = fr->get_channel(j);
        }
        ++i;
      }
      v.push_back(ary);
    }

    return v;
  }

  static std::vector<np::ndarray> get_channel(T & x)
  {
    np::dtype dtype = np::dtype::get_builtin<int>();

    std::vector<np::ndarray> v;
    for (const auto& [key, val] : x){
      std::cout << "key = " << key << std::endl;
      p::tuple shape = p::make_tuple(val.size(), 256);
      np::ndarray ary = np::empty(shape, dtype);
      int i = 0;
      for (const auto& fr : val) {
        for (int j = 0; j < 256; ++j) {
          ary[i][j] = fr->get_channel(j);
        }
        ++i;
      }
      v.push_back(ary);
    }

    return v;
  }

  static std::vector<np::ndarray> get_plane(T & x)
  {
    np::dtype dtype = np::dtype::get_builtin<int>();

    std::vector<np::ndarray> v;
    for (const auto& [key, val] : x){
      std::cout << "key = " << key << std::endl;
      p::tuple shape = p::make_tuple(val.size(), 256);
      np::ndarray ary = np::empty(shape, dtype);
      int i = 0;
      for (const auto& fr : val) {
        for (int j = 0; j < 256; ++j) {
          ary[i][j] = fr->get_channel(j);
          std::cout << key << " " << i << " " << j << " " << fr->get_channel(j) << std::endl;
        }
        ++i;
      }
      v.push_back(ary);
    }

    return v;
  }



  static void set(T const& x, K const& i, V const& v)
  {
      x[i]=v; // use map autocreation feature
  }
  static void del(T const& x, K const& i)
  {
      if( x.find(i) != x.end() ) x.erase(i);
      else KeyError();
  }
};

int main() {
  Py_Initialize();
  np::initialize();
  
  str name = str("hello");
  name.upper();
  std::string text = extract<std::string>(name.upper());
  std::cout << text << std::endl;
  np::ndarray a = np::zeros(p::make_tuple(3, 3), np::dtype::get_builtin<float>());
  std::cout << "Original array:\n" << p::extract<char const *>(p::str(a)) << std::endl;

  std::map<int, std::vector<fddetdataformats::WIBFrame*>> map_with_frames;
  std::vector<int> test;
  std::vector<fddetdataformats::WIBFrame> tmp;
  for (int i = 0; i < 256; ++i) {
    fddetdataformats::WIBFrame* fr = new fddetdataformats::WIBFrame;
    for (int j = 0; j < 256; ++j) {
      fr->set_channel(j, j);
    }
    map_with_frames[0].push_back(fr);
  }

  for (int i = 0; i < 256; ++i) {
    fddetdataformats::WIBFrame* fr = new fddetdataformats::WIBFrame;
    for (int j = 0; j < 256; ++j) {
      fr->set_channel(j, j+1);
    }
    map_with_frames[2].push_back(fr);
  }
  // object module = import("numpy");
  // object array = module.attr("array");
  object module;
  try {
    module = import("process");
  } catch (p::error_already_set& e) {
    std::cout << "Unable to import module" << std::endl;
  }

  typedef std::vector<fddetdataformats::WIBFrame*> vect;
  class_<std::vector<fddetdataformats::WIBFrame*>>("Frames")
    .def("__len__", &vect::size)
    .def("__getitem__", &std_item<vect>::get,
        return_value_policy<copy_non_const_reference>()
           )
    ;

  typedef std::map<int, std::vector<fddetdataformats::WIBFrame*>> mapt;
  class_<std::map<int, std::vector<fddetdataformats::WIBFrame*>>>("MapWithFrames")
    .def("__len__", &mapt::size)
    .def("__getitem__", &MapItem<mapt>::get
        // return_value_policy<copy_non_const_reference>()
         )
    .def("get_adc", &MapItem<mapt>::get_adc
        // return_value_policy<copy_non_const_reference>()
         )
      ;

  class_<std::vector<np::ndarray>>("VecClass")
    .def("__len__", &std::vector<np::ndarray>::size)
    .def("__getitem__", &std_item<std::vector<np::ndarray>>::get,
        return_value_policy<copy_non_const_reference>()
           )
    ;

  // class_<std::pair<np::ndarray, np::ndarray>>("TestClass");

    
  object f = module.attr("f");
  f(map_with_frames);
  // std::cout << "Original array:\n" << p::extract<char const *>(p::str(f(a))) << std::endl;

  // object narray = array.attr("__init__")(list());
  // std::cout << extract<int>(array.attr("size")) << std::endl;

  // object pyFunPlxMsgWrapper = import("numpy").attr("float");
  // cout << pyFunPlxMsgWrapper << std::endl;
  // std::cout << sizeof(float) << " " << sizeof(double) << std::endl;
  // std::complex<double> num;
  // num = {3.1, 4};
  // std::cout << std::real(num) << " " << std::imag(num) << " " << std::abs(num) << std::endl;

}
