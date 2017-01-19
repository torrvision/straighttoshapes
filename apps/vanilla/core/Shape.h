/**
 * vanilla: Shape.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_SHAPE
#define H_VANILLA_SHAPE

#include "VOCBox.h"

#include <ostream>
#include <opencv2/core/core.hpp>

/*
 * brief Represents a shape.
 */
class Shape
{
  //#################### PRIVATE MEMBER VARIABLES ####################
private:
  VOCBox m_box;
  cv::Mat1b m_mask;

  //#################### CONSTRUCTORS ####################
public:
  Shape();
  explicit Shape(const VOCBox& box);
  Shape(const VOCBox& box, const cv::Mat1b& mask);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  VOCBox get_voc_box() const;
  cv::Mat1b get_mask() const;

  float calculate_intersection_area(const Shape& shape, uint8_t binaryMaskThreshold = 100) const;
  float area(uint8_t binaryMaskThreshold = 100) const;

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  cv::Mat1b draw_binary_mask_on_canvas(const VOCBox& vbox, const cv::Mat1b& mask, size_t canvasWidth, size_t canvasHeight, uint8_t binaryMaskThreshold) const;
};

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const Shape& s);

#endif
