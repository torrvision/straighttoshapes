/**
 * vanilla: DataTransformation.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "DataTransformation.h"

#include <opencv2/imgproc/imgproc.hpp>

//#################### CONSTRUCTORS ####################

DataTransformation::DataTransformation(float rotation, float xTranslation, float yTranslation, float spatialScaleFactor, float scaleIntensityFactor, float addToIntensityValue, bool yflip, size_t imageWidthNetwork, size_t imageHeightNetwork)
: m_addToIntensityValue(addToIntensityValue),
  m_imageHeightNetwork(imageHeightNetwork),
  m_imageWidthNetwork(imageWidthNetwork),
  m_rotation(rotation),
  m_scaleIntensityFactor(scaleIntensityFactor),
  m_spatialScaleFactor(spatialScaleFactor),
  m_xTranslation(xTranslation),
  m_yflip(yflip),
  m_yTranslation(yTranslation)
{
  if(abs(m_rotation) > 4.0f)
  {
    throw std::runtime_error("Data Transformation currently designed to handle small rotations <= 2 degrees");
  }

  if(abs(1-spatialScaleFactor) > 0.01f)
  {
    throw std::runtime_error("Data transformation currently designed to handl small spatial scale changes <= 1%");
  }
}

//#################### PUBLIC MEMBER FUNCTIONS ####################

cv::Mat1b DataTransformation::apply_label_image_transformation(const cv::Mat1b& im) const
{
  cv::Mat1b result = im.clone();

  // Apply rotation.
  cv::warpAffine(result, result, calculate_rotation_matrix(im.cols, im.rows), result.size(), cv::INTER_NEAREST);

  // Apply translation.
  cv::warpAffine(result, result, calculate_translation_matrix(im.cols, im.rows), result.size(), cv::INTER_NEAREST);

  // Apply Flip.
  if(m_yflip) cv::flip(result, result, 1);

  return result;
}

cv::Mat3b DataTransformation::apply_real_image_transformation(const cv::Mat3b& im) const
{
  cv::Mat3b result = im.clone();

  // Small linear transformation of the pixel intensities.
  result = m_scaleIntensityFactor * result + m_addToIntensityValue;

  // Apply rotation.
  cv::warpAffine(result, result, calculate_rotation_matrix(im.cols, im.rows), result.size(), cv::INTER_LINEAR);

  // Apply translation.
  cv::warpAffine(result, result, calculate_translation_matrix(im.cols, im.rows), result.size(), cv::INTER_LINEAR);

  // Apply Flip.
  if(m_yflip) cv::flip(result, result, 1);

  // Apply resize.
  cv::Mat3b resultResized;
  cv::resize(result, resultResized, cv::Size(m_imageWidthNetwork, m_imageHeightNetwork));

  return resultResized;
}

VOCBox DataTransformation::apply_scale_and_clip_to_network_size(const VOCBox& vbox, const Size& imageSize) const
{
  VOCBox result(vbox);

  // Scale the box the input size of the network and clip to the image boundaries.
  float sx = (float)m_imageWidthNetwork/(float)imageSize.width;
  float sy = (float)m_imageHeightNetwork/(float)imageSize.height;
  result.scale(sx, sy);
  result.clip_to_image_boundaries(m_imageWidthNetwork, m_imageHeightNetwork);

  return result;
}

VOCBox DataTransformation::apply_transformation(const VOCBox& vbox, const Size& imageSize) const
{
  // TODO: For larger rotations, the box will need to be adjusted for rotation.
  VOCBox result(vbox);
  result.translate(calculate_x_translation(imageSize.width), calculate_y_translation(imageSize.height));
  if(m_yflip) result.flip_vertical(imageSize.width);

  return apply_scale_and_clip_to_network_size(result, imageSize);
}

//#################### PRIVATE MEMBER FUNCTIONS ####################

cv::Mat DataTransformation::calculate_rotation_matrix(int imageWidth, int imageHeight) const
{
  return cv::getRotationMatrix2D(cv::Point(imageHeight/2, imageWidth/2), m_rotation, m_spatialScaleFactor);
}

cv::Mat DataTransformation::calculate_translation_matrix(int imageWidth, int imageHeight) const
{
  cv::Mat translation = (cv::Mat_<float>(2,3) << 1, 0, calculate_x_translation(imageWidth),
                                                 0, 1, calculate_y_translation(imageHeight));
  return translation;
}

float DataTransformation::calculate_x_translation(int imageWidth) const
{
  return m_xTranslation * imageWidth;
}

float DataTransformation::calculate_y_translation(int imageHeight) const
{
  return m_yTranslation * imageHeight;
}

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const DataTransformation& d)
{
  os << "imageWidthNetwork" << d.m_imageWidthNetwork << '\n';
  os << "imageHeightNetwork" << d.m_imageHeightNetwork << '\n';
  os << "rotation: " << d.m_rotation << '\n';
  os << "xTranslation: " << d.m_xTranslation << '\n';
  os << "yTranslation: " << d.m_yTranslation << '\n';
  os << "addToIntensityValue: " << d.m_addToIntensityValue << '\n';
  os << "scaleIntensityFactor: " << d.m_scaleIntensityFactor << '\n';
  os << "spatialScaleFactor: " << d.m_spatialScaleFactor << '\n';
  os << "yflip: " << d.m_yflip << '\n';
  return os;
}
