/**
 * tvgshape: ShapeDescriptorUtil.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "ShapeDescriptorUtil.h"

namespace tvgshape {

//#################### PUBLIC STATIC MEMBER FUNCTIONS ####################

double ShapeDescriptorUtil::calculate_IoU(const cv::Mat1b& mask1, const cv::Mat1b& mask2)
{
  if(mask1.size() != mask2.size()) return 0.0;

  const unsigned char *p1 = mask1.data;
  const unsigned char *p2 = mask2.data;

  double Intersection = 0.0, Union = 0.0;
  for(size_t i = 0, size = mask1.rows * mask1.cols; i < size; ++i)
  {
    if(p1[i] && p2[i]) ++Intersection;
    if(p1[i] || p2[i]) ++Union;
  }

  return Intersection / Union;
}

bool ShapeDescriptorUtil::is_perfect_square(size_t k)
{
  double doublex = sqrt(k);
  int intx = static_cast<int>(doublex);
  if(abs((doublex*doublex) - static_cast<double>(intx*intx)) > 1e-5)
    return false;

  return true;
}

std::vector<float> ShapeDescriptorUtil::make_gray_image(const cv::Mat1b& im, float scaleFactor)
{
  int width = im.cols;
  int height = im.rows;
  std::vector<float> grayData(width*height);
  //float *grayData = new float[width*height];

  int counter(0);
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      float gray = static_cast<float>(im(y,x));
      grayData[counter] = gray * scaleFactor;
      ++counter;
    }
  }

  return grayData;
}


cv::Mat1b ShapeDescriptorUtil::make_gray_image(const std::vector<float>& grayData, int width, int height, float scaleFactor)
{
  cv::Mat1b result = cv::Mat1b::zeros(height, width);
  int counter(0);
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      float scaledValue = grayData[counter] * scaleFactor;
      if(scaledValue > 255.0f) scaledValue = 255.0f;
      if(scaledValue < 0.0f) scaledValue = 0.0f;
      result(y,x) = static_cast<unsigned char>(scaledValue);
      ++counter;
    }
  }
  return result;
}

}
