/**
 * vanilla: Box.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2015, All rights reserved.
 */

#include "Box.h"
#include "VOCBox.h"

#include <stdexcept>

//#################### CONSTRUCTORS ####################

Box::Box()
: x(-1.0f), y(-1.0f), w(-1.0f), h(-1.0f)
{}

Box::Box(float x_, float y_, float w_, float h_)
{
  if(x_ < 0.0f || y_ < 0.0f || w_ < 0.0f || h_ < 0.0f)
    throw std::runtime_error("Box cannot have negative values");

  x = x_; y = y_; w = w_; h = h_;
}

Box::Box(const VOCBox& b)
{
  w = static_cast<float>(b.xmax - b.xmin);
  h = static_cast<float>(b.ymax - b.ymin);
  x = b.xmin + w/2.0f;
  y = b.ymin + h/2.0f;
  Box(x, y, w, h);
}

//#################### PUBLIC MEMBER FUNCTIONS ####################

void Box::scale(float sx, float sy)
{
  x = x*sx;
  y = y*sy;
  w = w*sx;
  h = h*sy;
}

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const Box& b)
{
  os << "x: " << b.x << '\n';
  os << "y: " << b.y << '\n';
  os << "w: " << b.w << '\n';
  os << "h: " << b.h << '\n';
  return os;
}
