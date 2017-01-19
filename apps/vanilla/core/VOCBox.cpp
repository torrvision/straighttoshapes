/**
 * vanilla: VOCBox.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2015, All rights reserved.
 */

#include "VOCBox.h"
#include "Box.h"

#include <algorithm>
#include <stdexcept>

//#################### CONSTRUCTORS #################### 

VOCBox::VOCBox()
: xmin(-1), ymin(-1), xmax(-1), ymax(-1)
{}

VOCBox::VOCBox(int xmin_, int ymin_, int xmax_, int ymax_)
{
  if(xmin_ < 0 || ymin_ < 0 || xmax_ < 0 || ymax_ < 0)
    throw std::runtime_error("Box cannot have negative values");

  if(xmin_ > xmax_ || ymin_ > ymax_)
    throw std::runtime_error("Box cannot have negative with or height");

  xmin = xmin_; ymin = ymin_; xmax = xmax_; ymax = ymax_;
}

VOCBox::VOCBox(const Box& b)
{
  float halfWidth = b.w/2.0f;
  float halfHeight = b.h/2.0f;

  xmin = b.x - halfWidth;
  ymin = b.y - halfHeight;
  xmax = b.x + halfWidth;
  ymax = b.y + halfHeight;
  VOCBox(xmin < 0 ? 0 : xmin , ymin < 0 ? 0 : ymin, xmax, ymax);
}

//#################### PUBLIC MEMBER FUNCTIONS #################### 

int VOCBox::tlx() const
{
  return xmin;
}

int VOCBox::tly() const
{
  return ymin;
}

int VOCBox::w() const
{
  return xmax-xmin+1;
}

int VOCBox::h() const
{
  return ymax-ymin+1;
}

int VOCBox::area() const
{
  return w()*h();
}

float VOCBox::calculate_intersection_area(const VOCBox& b) const
{
  float iw = std::min(this->xmax, b.xmax) - std::max(this->xmin, b.xmin) + 1;
  float ih = std::min(this->ymax, b.ymax) - std::max(this->ymin, b.ymin) + 1;
  if(iw < 0 && ih < 0) return -iw*ih;
  else return iw*ih;
}

float VOCBox::overlap(const VOCBox& b) const
{
  float intersectionArea = calculate_intersection_area(b);
  float unionArea = area() + b.area() - intersectionArea;
  return intersectionArea / unionArea;
}

void VOCBox::clip_to_image_boundaries(int imageWidth, int imageHeight)
{
  if(xmin < 0) xmin = 0;
  if(ymin < 0) ymin = 0;
  if(xmax > imageWidth) xmax = imageWidth;
  if(ymax > imageHeight) ymax = imageHeight;

  // If the box is outside the image region, default initialise it to -1.
  if(xmin > (imageWidth-1) || ymin > (imageHeight-1) || xmax < 1 || ymax < 1)
    xmin = xmax = ymin = ymax = -1;
}

bool VOCBox::valid() const
{
  if(xmin < 0 || ymin < 0 || xmax < 0 || ymax < 0) return false;
  if(xmin == xmax || ymin == ymax) return false;

  return true;
}

void VOCBox::translate(int tx, int ty)
{
  xmin += tx;
  xmax += tx;
  ymin += ty;
  ymax += ty;
}

void VOCBox::scale(float sx, float sy)
{
  xmin = static_cast<int>(sx*(float)xmin);
  xmax = static_cast<int>(sx*(float)xmax);
  ymin = static_cast<int>(sy*(float)ymin);
  ymax = static_cast<int>(sy*(float)ymax);
}

void VOCBox::flip_vertical(int imageWidth)
{
  int xminCopy(xmin);
  xmin = imageWidth - xmax;
  xmax = imageWidth - xminCopy;
}

//#################### OUTPUT #################### 

std::ostream& operator<<(std::ostream& os, const VOCBox& b)
{
  os << "xmin: " << b.xmin << '\n';
  os << "ymin: " << b.ymin << '\n';
  os << "xmax: " << b.xmax << '\n';
  os << "ymax: " << b.ymax << '\n';
  return os;
}
