/**
 * vanilla: DetectionComparator.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "Detection.h"
#include "DetectionComparator.h"

//#################### CONSTRUCTORS ####################

template <typename DT>
DetectionComparator<DT>::DetectionComparator(int categoryId, const boost::function<bool(float, float)>& comp)
: m_categoryId(categoryId),
  m_comp(comp)
{}

//#################### PUBLIC MEMBER FUNCTIONS ####################

template <typename DT>
bool DetectionComparator<DT>::operator()(const DT& a, const DT& b) const
{
  return m_comp(a.second[m_categoryId], b.second[m_categoryId]);
}

//#################### EXPLICIT INSTANTIATIONS ####################

template class DetectionComparator<Detection>;
