/**
 * vanilla: TupleComparator.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_TUPLECOMPARATOR
#define H_VANILLA_TYPLECOMPARATOR

#include <vector>

#include <boost/function.hpp>
#include <boost/tuple/tuple.hpp>

template <int M, typename T>
class TupleComparator
{
  //#################### PRIVATE MEMBER VARIABLES ####################
private:
  boost::function<bool(float, float)> m_comp;

  //#################### CONSTRUCTORS ####################
public:
  TupleComparator(const boost::function<bool(float, float)>& comp)
  : m_comp(comp)
  {}

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  bool operator()(const T& a, const T& b) const
  {
    return m_comp(boost::get<M>(a), boost::get<M>(b));
  }
};

#endif
