/**
 * tvgshape: ShapeDescriptorUtil.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_TVGSHAPE_SHAPEDESCRIPTORUTIL
#define H_TVGSHAPE_SHAPEDESCRIPTORUTIL

#include <opencv2/opencv.hpp>

namespace tvgshape {

/**
 * \brief TODO
 */
struct ShapeDescriptorUtil
{
  //#################### PUBLIC STATIC MEMBER FUNCTIONS ####################

  /**
   * \brief TODO
   */
  static double calculate_IoU(const cv::Mat1b& mask1, const cv::Mat1b& mask2);
  static std::vector<float> make_gray_image(const cv::Mat1b& im, float scaleFactor);
  static cv::Mat1b make_gray_image(const std::vector<float>& grayData, int width, int height, float scaleFactor);
  static bool is_perfect_square(size_t k);
};

}

#endif
