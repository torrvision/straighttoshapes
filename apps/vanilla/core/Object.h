/**
 * vanilla: Object.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_OBJECT
#define H_VANILLA_OBJECT

#include <iostream>

template <typename Rep>
class Object
{
  //#################### PUBLIC MEMBER VARIABLES #################### 
public:
  int categoryId;
  Rep rep;

  //#################### CONSTRUCTORS #################### 
public:
  Object(const Rep& rep_, int categoryId_ = -1)
  : categoryId(categoryId_),
    rep(rep_)
  {}
};

//#################### OUTPUT #################### 
template <typename Rep>
std::ostream& operator<<(std::ostream& os, const Object<Rep>& o)
{
  os << "category: " << o.categoryId << '\n';
  os << "representation: " << o.rep << '\n';
  return os;
}

#endif
