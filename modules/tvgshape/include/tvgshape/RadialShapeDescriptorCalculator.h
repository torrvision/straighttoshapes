/**
 * tvgshape: RadialShapeDescriptorCalculator.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_TVGSHAPE_RADIALSHAPEDESCRIPTORCALCULATOR
#define H_TVGSHAPE_RADIALSHAPEDESCRIPTORCALCULATOR

#include "ShapeDescriptorCalculator.h"

namespace tvgshape {

/**
 * \brief TODO
 */
class RadialShapeDescriptorCalculator : public ShapeDescriptorCalculator
{
  //#################### ENUMERATIONS ####################
private:
  /**
   * \brief TODO
   */
  enum Mode
  {
    MODE_INSIDEOUT,
    MODE_OUTSIDEIN,
    MODE_COUNT
  };

  //#################### PRIVATE VARIABLES ####################
private:
  /** TODO */
  int m_gridSize;

  /** TODO */
  int m_medianKernelSize;

  /** TODO */
  int m_outputMaskSize;

  //#################### CONSTRUCTORS ####################
public:
  /**
   * \brief TODO
   */
  RadialShapeDescriptorCalculator(int gridSize = 5, int medianKernelSize = 0, int outputMaskSize = 512);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  /** Override */
  virtual std::vector<float> from_mask(const cv::Mat1b& originalMask, size_t descriptorSize) const;

  /** Override */
  virtual cv::Mat1b to_mask(const std::vector<float>& descriptor, const cv::Size& maskSize) const;

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  /**
   * \brief TODO
   */
  std::vector<float> from_mask(const cv::Mat1b& mask, size_t descriptorSize, Mode mode, float cx, float cy) const;
};

}

#endif
