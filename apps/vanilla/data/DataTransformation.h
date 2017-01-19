/**
 * vanilla: DataTransformation.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_DATATRANSFORMATION
#define H_VANILLA_DATATRANSFORMATION

#include "../core/Detection.h"
#include "../core/Size.h"

#include <opencv2/core/core.hpp>

class DataTransformation
{
  //#################### PRIVATE VARIABLES ####################
private:
  float m_addToIntensityValue;
  size_t m_imageHeightNetwork;
  size_t m_imageWidthNetwork;
  float m_rotation;
  float m_scaleIntensityFactor;
  float m_spatialScaleFactor;
  float m_xTranslation;
  bool m_yflip;
  float m_yTranslation;

  //#################### CONSTRUCTORS ####################
public:
  DataTransformation(float rotation, float xTranslation, float yTranslation, float spatialScaleFactor, float scaleIntensityFactor, float addToIntensityValue, bool yflip, size_t imageWidthNetwork, size_t imageHeightNetwork);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  cv::Mat1b apply_label_image_transformation(const cv::Mat1b& im) const;

  cv::Mat3b apply_real_image_transformation(const cv::Mat3b& im) const;

  VOCBox apply_scale_and_clip_to_network_size(const VOCBox& vbox, const Size& imageSize) const;

  VOCBox apply_transformation(const VOCBox& vbox, const Size& imageSize) const;


  friend std::ostream& operator<<(std::ostream& os, const DataTransformation& d);

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  cv::Mat calculate_rotation_matrix(int imageWidth, int imageHeight) const;
  cv::Mat calculate_translation_matrix(int imageWidth, int imageHeight) const;
  float calculate_x_translation(int imageWidth) const;
  float calculate_y_translation(int imageHeight) const;
};

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const DataTransformation& d);

#endif


