/**
 * @file PythonUtils.hpp Define some constants used by DQM
 *
 * This is part of the DUNE DAQ , copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */
#ifndef DQM_INCLUDE_PYTHONUTILS_HPP_
#define DQM_INCLUDE_PYTHONUTILS_HPP_

#include <string>

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

#include "dqm/ChannelMap.hpp"

namespace dunedaq::dqm {

namespace p = boost::python;
namespace np = boost::python::numpy;

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
  // std::shared_ptr<std::map<int, std::vector<detdataformats::wib::WIBFrame*>>> ptr;
  // MapItem (std::map<int, std::vector<detdataformats::wib::WIBFrame*>> map_with_frames) {
  //   ptr = std::make_shared<std::map<int, std::vector<detdataformats::wib::WIBFrame*>>>(map_with_frames);
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


  static std::vector<np::ndarray> get_channels(T& x, std::shared_ptr<ChannelMap>& cmap)
  {
    np::dtype dtype = np::dtype::get_builtin<int>();

    std::vector<np::ndarray> v;
    for (const auto& [key, val] : x){
      std::vector<int> channels;
      for (int j = 0; j < 256; ++j) {
        if (cmap->m_map_rev.find(key) != cmap->m_map_rev.end() && cmap->m_map_rev[key].find(j) != cmap->m_map_rev[key].end()) {
          channels.push_back(cmap->m_map_rev[key][j].second);
        }
      }
      np::ndarray ary = np::empty(p::make_tuple(channels.size()), dtype);
      for (int i = 0; i < channels.size(); i++) {
        ary[i] = channels[i];
      }
      // std::copy(channels.begin(), channels.end(), reinterpret_cast<int*>(ary.get_data()));
      v.push_back(ary);
    }
    return v;
  }


  static std::vector<np::ndarray> get_planes(T & x, std::shared_ptr<ChannelMap>& cmap)
  {
    np::dtype dtype = np::dtype::get_builtin<int>();


    std::vector<np::ndarray> v;
    for (const auto& [key, val] : x){
      std::vector<int> planes;
      for (int j = 0; j < 256; ++j) {
        if (cmap->m_map_rev.find(key) != cmap->m_map_rev.end() && cmap->m_map_rev[key].find(j) != cmap->m_map_rev[key].end()) {
          planes.push_back(cmap->m_map_rev[key][j].first);
        }
      }
      np::ndarray ary = np::empty(p::make_tuple(planes.size()), dtype);
      for (int i = 0; i < planes.size(); i++) {
        ary[i] = planes[i];
      }
      // std::copy(planes.begin(), planes.end(), reinterpret_cast<int*>(ary.get_data()));
      v.push_back(ary);
    }
    return v;
  }


};


} // namespace dunedaq::dqm

#endif // DQM_INCLUDE_PYTHONUTILS_HPP_
