/**
 * vanilla: InputDataAssembler.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "InputDataAssembler.h"


//#################### CONSTRUCTORS ####################

InputDataAssembler::InputDataAssembler(const std::vector<std::string>& imagePaths, CQ_Ptr& cq, size_t imageWidthNetwork, size_t imageHeightNetwork)
: m_cq(cq),
  m_imagePaths(imagePaths),
  m_imageWidthNetwork(imageWidthNetwork),
  m_imageHeightNetwork(imageHeightNetwork)
{
  // Initialise the first element on construction.
  std::pair<cv::Mat3b,cv::Size> data = read_and_resize(m_imagePaths[0]);
  if(!m_cq->push(data)) throw std::runtime_error("Could not load first image into circular queue");
}

//#################### PUBLIC MEMBER FUNCTIONS ####################

void InputDataAssembler::run_load_loop()
{
  const size_t pathCount = m_imagePaths.size();
  for(size_t i = 1; i < pathCount; ++i)
  {
    std::pair<cv::Mat3b, cv::Size> data = read_and_resize(m_imagePaths[i]);

    bool loaded = false;
    while(!loaded)
    {
      loaded = m_cq->push(data);
    }
  }
}

//#################### PRIVATE MEMBER FUNCTIONS ####################

std::pair<cv::Mat3b,cv::Size> InputDataAssembler::read_and_resize(const std::string& path)
{
  cv::Mat3b im = cv::imread(path, CV_LOAD_IMAGE_COLOR);
  cv::Mat3b resized(im.cols, im.rows);
  cv::resize(im, resized, cvSize(m_imageWidthNetwork, m_imageHeightNetwork));

  return std::make_pair(resized,CvSize(im.cols, im.rows));
}

