/**
 * vanilla: VOCObject.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_VOCOBJECT
#define H_VANILLA_VOCOBJECT

#include <stdexcept>

#include "Object.h"
#include "Shape.h"

class VOCObject : public Object<Shape>
{
  //#################### PUBLIC MEMBER VARIABLES #################### 
public:
  bool difficult;
  std::string categoryName;

  //#################### CONSTRUCTORS #################### 
public:
  VOCObject(bool difficult_, const Shape& shape_, const std::string& categoryName_, size_t categoryId_);
};

std::ostream& operator<<(std::ostream& os, const VOCObject& o);

#endif
