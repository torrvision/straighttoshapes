/**
 * vanilla: Size.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2015, All rights reserved.
 */

#include "Size.h"

#include <stdexcept>

//#################### CONSTRUCTORS #################### 

Size::Size()
: width(-1), height(-1), depth(-1)
{}

Size::Size(int width_, int height_, int depth_)
{
  if(width_ < 0 || height_ < 0 || depth_ < 0)
    throw std::runtime_error("Size cannot have negative values");

  width = width_; height = height_; depth = depth_;
}

//#################### OUTPUT #################### 
std::ostream& operator<<(std::ostream& os, const Size& s)
{
  os << "width: " << s.width << '\n';
  os << "height: " << s.height << '\n';
  os << "depth: " << s.depth << '\n';
  return os;
}
