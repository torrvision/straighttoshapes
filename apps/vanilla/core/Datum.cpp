/**
 * vanilla: Datum.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016, All rights reserved.
 */

#include "Datum.h"

#include <tvgutil/containers/LimitedContainer.h>
using namespace tvgutil;

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const Datum& d)
{
  std::cout << "input:\n" << make_limited_container(d.first, 100) << '\n';
  std::cout << "output:\n" << make_limited_container(d.second, 100) << '\n';

  return os;
}
