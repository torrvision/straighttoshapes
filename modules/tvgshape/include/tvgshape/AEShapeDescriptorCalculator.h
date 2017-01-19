/**
 * tvgshape: AEShapeDescriptorCalculator.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_TVGSHAPE_AESHAPEDESCRIPTORCALCULATOR
#define H_TVGSHAPE_AESHAPEDESCRIPTORCALCULATOR

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <LuaBridge.h>

#include <boost/thread.hpp>

#include "ShapeDescriptorCalculator.h"

namespace tvgshape {

/**
 * \brief TODO
 */
class AEShapeDescriptorCalculator : public ShapeDescriptorCalculator
{
  //#################### PRIVATE VARIABLES ####################
private:
  /** TODO */
  int m_binaryMaskThreshold;

  /** TODO */
  int m_descriptorSize;

  /** TODO */
  lua_State *m_L;

  mutable boost::mutex m_mtx;

  //#################### CONSTRUCTORS ####################
public:
  /**
   * \brief TODO
   */
  AEShapeDescriptorCalculator(lua_State *L, const std::string& scriptFilename, const std::string& modelDir, int descriptorSize, int binaryMaskThreshold = 128);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  /** Override */
  virtual std::vector<float> from_mask(const cv::Mat1b& mask, size_t descriptorSize) const;

  /** Override */
  virtual cv::Mat1b to_mask(const std::vector<float>& descriptor, const cv::Size& maskSize) const;

  //#################### PRIVATE STATIC MEMBER FUNCTIONS ####################
private:
  template <typename T>
  static T tableToMap(const luabridge::LuaRef& table)
  {
    T map;
    if(table.isTable())
    {
      for(luabridge::Iterator iter(table); !iter.isNil(); ++iter)
      {
        map[iter.key()] = iter.value(); // implicit conversion from LuaRefs should be called here
      }
    }
    return map;
  }

  template <typename T>
  static std::vector<T> tableToVector(const luabridge::LuaRef& table)
  {
    std::map<int,T> m = tableToMap<std::map<int,T> >(table);
    std::vector<float> vec(m.size());
    for(typename std::map<int,T>::const_iterator it = m.begin(), iend = m.end(); it != iend; ++it)
    {
      if(it->first >= 1 && it->first <= vec.size())
      {
        vec[it->first - 1] = it->second;
      }
    }
    return vec;
  }

  template <typename T>
  static luabridge::LuaRef vectorToTable(lua_State *L, const std::vector<T>& vec)
  {
    luabridge::LuaRef table = luabridge::newTable(L);
    for(size_t i = 0, size = vec.size(); i < size; ++i)
    {
      table[(int)(i + 1)] = vec[i];
    }
    return table;
  }

  //#################### NESTED TYPES ####################
private:
  class FVTransformer
  {
  private:
    luabridge::LuaRef m_func;
    lua_State *m_L;

  public:
    FVTransformer(lua_State *L, const std::string& funcName)
    : m_func(luabridge::getGlobal(L, funcName.c_str())), m_L(L)
    {}

  public:
    std::vector<float> operator()(const std::vector<float>& inVec, int descSize) const
    {
      return tableToVector<float>(m_func(vectorToTable(m_L, inVec), descSize));
    }
  };
};

}

#endif
