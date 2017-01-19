/**
 * vanilla: VOCBox.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2015. All rights reserved.
 */

#ifndef H_VANILLA_VOCBOX
#define H_VANILLA_VOCBOX

#include <ostream>

class Box;

class VOCBox
{
  //#################### PUBLIC MEMBER VARIABLES #################### 
public:
  int xmin, ymin, xmax, ymax;

  //#################### CONSTRUCTORS #################### 
public:
  VOCBox();
  VOCBox(int xmin_, int ymin_, int xmax_, int ymax_);
  explicit VOCBox(const Box& b);

  //#################### PUBLIC MEMBER FUNCTIONS #################### 
public:
  /* Return the top left x coordinate of the box. */
  int tlx() const;

  /* Return the top left y coordinate of the box. */
  int tly() const;

  /* Return the width of the box. */
  int w() const;

  /* Return the height of the box. */
  int h() const;

  /* Calculate the area of the box. */
  int area() const;

  /* Calculate the intersection over are of the box with another box. */
  float calculate_intersection_area(const VOCBox& b) const;

  /* Calculate the overlap with a specified box. */
  float overlap(const VOCBox& b) const;

  /* Clip the box to the boundaries of an image frame. */
  void clip_to_image_boundaries(int imageWidth, int imageHeight);

  void scale(float sx, float sy);

  /* Translate the box. */
  void translate(int tx, int ty);

  /* Flip the box in the vertical axes (y-coordinate). */
  void flip_vertical(int imageWidth);

  /* Checks whether the box has been initialised/valid or not. */
  bool valid() const;
};

//#################### OUTPUT #################### 

std::ostream& operator<<(std::ostream& os, const VOCBox& b);

#endif
