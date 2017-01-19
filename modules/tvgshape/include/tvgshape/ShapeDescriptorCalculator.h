/**
 * tvgshape: ShapeDescriptorCalculator.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_TVGSHAPE_SHAPEDESCRIPTORCALCULATOR
#define H_TVGSHAPE_SHAPEDESCRIPTORCALCULATOR

#include <vector>

#include <opencv2/opencv.hpp>
#include <boost/shared_ptr.hpp>

namespace tvgshape {

/**
 * \brief TODO
 */
class ShapeDescriptorCalculator
{
  //#################### DESTRUCTOR ####################
public:
  /**
   * \brief Destroys the shape descriptor calculator.
   */
  virtual ~ShapeDescriptorCalculator() {}

  //#################### PUBLIC ABSTRACT MEMBER FUNCTIONS ####################
public:
  /**
   * \brief TODO
   */
  virtual std::vector<float> from_mask(const cv::Mat1b& mask, size_t descriptorSize) const = 0;

  /**
   * \brief TODO
   */
  virtual cv::Mat1b to_mask(const std::vector<float>& descriptor, const cv::Size& maskSize) const = 0;
};

typedef boost::shared_ptr<ShapeDescriptorCalculator> ShapeDescriptorCalculator_Ptr;
typedef boost::shared_ptr<const ShapeDescriptorCalculator> ShapeDescriptorCalculator_CPtr;

}

#endif
