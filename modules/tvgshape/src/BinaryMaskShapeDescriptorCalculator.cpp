/**
 * tvgshape: BinaryMaskShapeDescriptorCalculator.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "BinaryMaskShapeDescriptorCalculator.h"

#include "ShapeDescriptorUtil.h"

namespace tvgshape {

//#################### CONSTRUCTORS ####################

BinaryMaskShapeDescriptorCalculator::BinaryMaskShapeDescriptorCalculator(int binaryMaskThreshold)
: m_binaryMaskThreshold(binaryMaskThreshold)
{}

//#################### PUBLIC MEMBER FUNCTIONS ####################

std::vector<float> BinaryMaskShapeDescriptorCalculator::from_mask(const cv::Mat1b& mask, size_t descriptorSize) const
{
  if(!ShapeDescriptorUtil::is_perfect_square(descriptorSize)) throw std::runtime_error("The descriptor size must be a perfect square");

  int k = static_cast<int>(sqrt(descriptorSize));
  cv::Mat1b resizedMask(k,k);
  cv::resize(mask, resizedMask, cv::Size(k,k), 0.0, 0.0, cv::INTER_AREA);

  return ShapeDescriptorUtil::make_gray_image(resizedMask, 1/255.0f);
}

cv::Mat1b BinaryMaskShapeDescriptorCalculator::to_mask(const std::vector<float>& descriptor, const cv::Size& maskSize) const
{
  size_t descriptorSize = descriptor.size();

  if(!ShapeDescriptorUtil::is_perfect_square(descriptorSize)) throw std::runtime_error("The descriptor size must be a perfect square");

  int k = static_cast<int>(sqrt(descriptorSize));
  cv::Mat1b outputMask = ShapeDescriptorUtil::make_gray_image(descriptor, k, k, 255.0f);

  cv::Mat1b finalMask;
  cv::resize(outputMask, finalMask, maskSize, 0.0, 0.0, cv::INTER_CUBIC);
  cv::threshold(finalMask, finalMask, m_binaryMaskThreshold, 255, cv::THRESH_BINARY);
  return finalMask;
}

}
