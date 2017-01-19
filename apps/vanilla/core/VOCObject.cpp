/**
 * vanilla: VOCObject.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2015, All rights reserved.
 */

#include "VOCObject.h"

#include <string>

//#################### CONSTRUCTORS #################### 

VOCObject::VOCObject(bool difficult_, const Shape& shape_, const std::string& categoryName_, size_t categoryId_)
: VOCObject::Object<Shape>(shape_, categoryId_),
  difficult(difficult_),
  categoryName(categoryName_)
{}

//#################### OUTPUT #################### 

std::ostream& operator<<(std::ostream& os, const VOCObject& o)
{
  os << "difficult: " << o.difficult << '\n';
  os << "category: " << o.categoryId << ", " << o.categoryName << '\n';
  os << "box: " << o.rep << '\n';
  return os;
}
