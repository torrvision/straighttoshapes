/**
 * vanilla: Detection.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2015, All rights reserved.
 */

#include "Detection.h"

#include <boost/tuple/tuple_io.hpp>

#include <tvgutil/containers/LimitedContainer.h>
using namespace tvgutil;

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const Detections& d)
{
  for(size_t i = 0, size = d.size(); i < size; ++i)
  {
    os << "Detection: " << i << '\n';
    os << d[i].first.get_voc_box() << '\n';
    os << make_limited_container(d[i].second, 20) << '\n';
    os << '\n';
  }

  return os;
}

std::ostream& operator<<(std::ostream& os, const NamedCategoryDetections& d)
{
  for(size_t i = 0, size = d.size(); i < size; ++i)
  {
    os << "Detection: " << i << ' ' << d[i].get<0>() << ' ';
    os << d[i].get<1>().get_voc_box() << ' ';
    os << d[i].get<2>() << '\n';
  }

  return os;
}
