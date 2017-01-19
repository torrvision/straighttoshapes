/**
 * vanilla: Shape.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "Shape.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../Util.h"

//#################### CONSTRUCTORS ####################

Shape::Shape()
{}

Shape::Shape(const VOCBox& box)
: m_box(box)
{}

Shape::Shape(const VOCBox& box, const cv::Mat1b& mask)
: m_box(box),
  m_mask(mask)
{}

//#################### PUBLIC MEMBER FUNCTIONS ####################

VOCBox Shape::get_voc_box() const
{
  return m_box;
}

cv::Mat1b Shape::get_mask() const
{
  return m_mask;
}

float Shape::calculate_intersection_area(const Shape& shape, uint8_t binaryMaskThreshold) const
{
  float boxIntersectionArea = m_box.calculate_intersection_area(shape.get_voc_box());
  if(boxIntersectionArea <= 0.0f || (!m_mask.data)) return boxIntersectionArea;

  // Only calculate the shape intersection area if the box intersection area is greater than zero, and a shape mask exists.

  // Calculate the maximum width or height the canvas needs to be.
  int xmax = std::max(m_box.xmax, shape.get_voc_box().xmax);
  int ymax = std::max(m_box.ymax, shape.get_voc_box().ymax);

  cv::Mat1b canvasA = draw_binary_mask_on_canvas(m_box, m_mask, xmax, ymax, binaryMaskThreshold);
  cv::Mat1b canvasB = draw_binary_mask_on_canvas(shape.get_voc_box(), shape.get_mask(), xmax, ymax, binaryMaskThreshold);

  cv::Mat1b intersectionMask = cv::Mat::zeros(ymax, xmax, CV_8UC1);
  cv::bitwise_and(canvasA, canvasB, intersectionMask);

//#define SHAPE_DRAW_MASK_DEBUG
#ifdef SHAPE_DRAW_MASK_DEBUG
  cv::imshow("mask", m_mask);

  cv::imshow("canvasA", canvasA);
  cv::imshow("canvasB", canvasB);

  cv::imshow("intersection", intersectionMask);
  cv::waitKey();
#endif

  return static_cast<float>(cv::countNonZero(intersectionMask));
}

float Shape::area(uint8_t binaryMaskThreshold) const
{
  float boxArea = m_box.area();
  if(boxArea <= 0.0f) throw std::runtime_error("The box area should not be less than or equal to zero");
  if(!m_mask.data) return boxArea;

  // Only calculate the shape intersection area if the box intersection area is greater than zero, and a shape mask exists.
  cv::Mat1b canvas = draw_binary_mask_on_canvas(m_box, m_mask, m_box.xmax, m_box.ymax, binaryMaskThreshold);
  return static_cast<float>(cv::countNonZero(canvas));
}

//#################### PRIVATE MEMBER FUNCTIONS ####################

cv::Mat1b Shape::draw_binary_mask_on_canvas(const VOCBox& vbox, const cv::Mat1b& mask, size_t canvasWidth, size_t canvasHeight, uint8_t binaryMaskThreshold) const
{
    // FIXME duplicate code in DetecitonUtil
    cv::Rect rect = Util::to_rect(vbox);
    cv::Mat1b resizedMask(rect.height, rect.width);
    if(mask.cols != rect.width || mask.rows != rect.height)
    {
      cv::resize(mask, resizedMask, resizedMask.size(), 0.0, 0.0, cv::INTER_CUBIC);
    }
    else
    {
      resizedMask = mask;
    }

    //cv::threshold(resizedMask, resizedMask, binaryMaskThreshold, 255, cv::THRESH_BINARY);

    // Create a canvas on which to draw the shapes.
    cv::Mat1b canvas = cv::Mat::zeros(canvasHeight, canvasWidth, CV_8UC1);
    cv::Mat1b roiCanvas = canvas(rect);
    cv::add(roiCanvas, resizedMask, roiCanvas, resizedMask);

    return canvas;
}

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const Shape& s)
{
  os << s.get_voc_box() << '\n';
  return os;
}
