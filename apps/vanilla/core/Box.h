/**
 * vanilla: Box.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2015. All rights reserved.
 */

#ifndef H_VANILLA_BOX
#define H_VANILLA_BOX

#include <ostream>

class VOCBox;

class Box
{
  //#################### PUBLIC MEMBER VARIABLES ####################
  /*
   * brief Represents a box with centre (x,y) and width and height w and h.
   */
public:
  float x, y, w, h;

  //#################### CONSTRUCTORS ####################
public:
  Box();
  Box(float x_, float y_, float w_, float h_);
  explicit Box(const VOCBox& b);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  void scale(float sx, float sy);
};

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const Box& b);

#endif
