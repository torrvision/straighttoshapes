/**
 * tvgshape: BinaryMaskShapeDescriptorCalculator.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_TVGSHAPE_BINARYMASKSHAPEDESCRIPTORCALCULATOR
#define H_TVGSHAPE_BINARYMASKSHAPEDESCRIPTORCALCULATOR

#include "ShapeDescriptorCalculator.h"

namespace tvgshape {

/**
 * \brief TODO
 */
class BinaryMaskShapeDescriptorCalculator : public ShapeDescriptorCalculator
{
  //#################### PRIVATE VARIABLES ####################
private:
  int m_binaryMaskThreshold;

  //#################### CONSTRUCTORS ####################
public:
  /**
   * \brief TODO
   */
  BinaryMaskShapeDescriptorCalculator(int binaryMaskThreshold = 128);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  /** Override */
  virtual std::vector<float> from_mask(const cv::Mat1b& mask, size_t descriptorSize) const;

  /** Override */
  virtual cv::Mat1b to_mask(const std::vector<float>& descriptor, const cv::Size& maskSize) const;
};

}

#endif
