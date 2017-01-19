/**
 * vanilla: DetectionComparator.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_DETECTIONCOMPARATOR
#define H_VANILLA_DETECTIONCOMPARATOR

#include <vector>

#include <boost/function.hpp>

template <typename DT>
class DetectionComparator
{
  //#################### PRIVATE MEMBER VARIABLES ####################
private:
  int m_categoryId;
  boost::function<bool(float, float)> m_comp;

  //#################### CONSTRUCTORS ####################
public:
  DetectionComparator(int categoryId, const boost::function<bool(float, float)>& comp);
#if 0
  : m_categoryId(categoryId),
    m_comp(comp)
  {}
#endif

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  bool operator()(const DT& a, const DT& b) const;
#if 0
  {
    return m_comp(a.second[m_categoryId], b.second[m_categoryId]);
  }
#endif
};

#endif
