/**
 * tvgshape: RadialShapeDescriptorCalculator.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "RadialShapeDescriptorCalculator.h"

#include "ShapeDescriptorUtil.h"

namespace tvgshape {

//#################### CONSTRUCTORS ####################

RadialShapeDescriptorCalculator::RadialShapeDescriptorCalculator(int gridSize, int medianKernelSize, int outputMaskSize)
: m_gridSize(gridSize), m_medianKernelSize(medianKernelSize), m_outputMaskSize(outputMaskSize)
{}

//#################### PUBLIC MEMBER FUNCTIONS ####################

std::vector<float> RadialShapeDescriptorCalculator::from_mask(const cv::Mat1b& originalMask, size_t descriptorSize) const
{
  int inputMaskSize = std::min(originalMask.rows, originalMask.cols);
  cv::Mat resizedOriginalMask;
  cv::resize(originalMask, resizedOriginalMask, cv::Size(inputMaskSize, inputMaskSize), CV_INTER_NN);

  cv::Mat1b inputMask = resizedOriginalMask > 0;
  if(m_medianKernelSize > 0) cv::medianBlur(inputMask, inputMask, m_medianKernelSize);

  std::vector<cv::Point2f> centres;
  float gridStep = 1.0f / (m_gridSize + 1);
  for(int i = 0; i < m_gridSize; ++i)
  {
    for(int j = 0; j < m_gridSize; ++j)
    {
      centres.push_back(cv::Point2f((j + 1) * gridStep, (i + 1) * gridStep));
    }
  }

  double bestIoU = 0.0;
  std::vector<float> bestDescriptor;
  cv::Mat1b bestFinalMask;
  for(int mode = 0; mode < MODE_COUNT; ++mode)
  {
    for(size_t i = 0, centreCount = centres.size(); i < centreCount; ++i)
    {
      std::vector<float> descriptor = from_mask(inputMask, descriptorSize, static_cast<Mode>(mode), centres[i].x, centres[i].y);
      cv::Mat1b finalMask = to_mask(descriptor, originalMask.size());

      double IoU = ShapeDescriptorUtil::calculate_IoU(originalMask, finalMask);
      if(IoU >= bestIoU)
      {
        bestIoU = IoU;
        bestDescriptor = descriptor;
        bestFinalMask = finalMask;
      }
    }
  }

  return bestDescriptor;
}

cv::Mat1b RadialShapeDescriptorCalculator::to_mask(const std::vector<float>& descriptor, const cv::Size& maskSize) const
{
  cv::Mat1b outputMask = cv::Mat1b::zeros(cv::Size(m_outputMaskSize, m_outputMaskSize));

  const size_t descriptorSize = descriptor.size();
  const float halfMaskSize = m_outputMaskSize / 2.0f;
  const float denom = 10.0f * sqrt(2.0f) * halfMaskSize;

  std::vector<std::vector<cv::Point2i> > contours;
  contours.push_back(std::vector<cv::Point2i>());
  std::vector<cv::Point2i>& contour = contours.back();

  const size_t angleCount = descriptorSize - 2;
  const float angleStep = static_cast<float>(M_PI * 2 / angleCount);
  for(size_t i = 0; i < angleCount; ++i)
  {
    float angle = i * angleStep;
    float dx = cos(angle), dy = sin(angle);
    float x = descriptor[0] * m_outputMaskSize + denom * descriptor[i+2] * dx;
    float y = descriptor[1] * m_outputMaskSize + denom * descriptor[i+2] * dy;
    int ix = static_cast<int>(x), iy = static_cast<int>(y);
    if(ix < 0) ix = 0;
    if(ix > m_outputMaskSize - 1) ix = m_outputMaskSize - 1;
    if(iy < 0) iy = 0;
    if(iy > m_outputMaskSize - 1) iy = m_outputMaskSize - 1;
    contour.push_back(cv::Point2i(ix, iy));
  }

  cv::fillPoly(outputMask, contours, cv::Scalar(255));
#if 0
  for(size_t i = 0, size = contour.size(); i < size; ++i)
  {
    outputMask.data[contour[i].y * outputMaskSize + contour[i].x] = 127;
  }
#endif

  cv::Mat1b finalMask;
  cv::resize(outputMask, finalMask, maskSize);
  finalMask = finalMask > 0;

  return finalMask;
}

//#################### PRIVATE MEMBER FUNCTIONS ####################

std::vector<float> RadialShapeDescriptorCalculator::from_mask(const cv::Mat1b& mask, size_t descriptorSize, Mode mode, float cx, float cy) const
{
  std::vector<float> descriptor(descriptorSize);
  descriptor[0] = cx;
  descriptor[1] = cy;

  const int maskSize = mask.cols;
  const float halfMaskSize = maskSize / 2.0f;
  const float denom = 10.0f * sqrt(2.0f) * halfMaskSize;

  const size_t angleCount = descriptorSize - 2;
  const float angleStep = static_cast<float>(M_PI * 2 / angleCount);
  for(size_t i = 0; i < angleCount; ++i)
  {
    float angle = i * angleStep;
    float dx = cos(angle), dy = sin(angle);
    float x = cx * maskSize, y = cy * maskSize;
    float value = -1.0f;
    unsigned char lastMaskVal = 0;
    for(int j = 0;; ++j)
    {
      int ix = static_cast<int>(x), iy = static_cast<int>(y);

      if(ix < 0 || ix >= maskSize || iy < 0 || iy >= maskSize)
      {
        if(value < 0.0f) value = static_cast<float>(j - 1);
        break;
      }

      unsigned char maskVal = mask.data[iy * maskSize + ix];
      if(lastMaskVal != 0 && maskVal == 0)
      {
        value = static_cast<float>(j - 1);
        if(mode == MODE_INSIDEOUT) break;
      }

      lastMaskVal = maskVal;
      x += dx, y += dy;
    }

    descriptor[i + 2] = value / denom;
    if(descriptor[i + 2] < 0.0f) descriptor[i + 2] = 0.0f;
    if(descriptor[i + 2] > 1.0f) descriptor[i + 2] = 1.0f;
  }

  return descriptor;
}

}
