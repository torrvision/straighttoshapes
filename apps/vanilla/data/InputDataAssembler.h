/**
 * vanilla: InputDataAssembler.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_INPUTIMAGEASSEMBLER
#define H_VANILLA_INPUTIMAGEASSEMBLER

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <tvgutil/containers/CircularQueue.h>

/**
 * \brief TODO.
 */
class InputDataAssembler
{
  //#################### PRIVATE TYPEDEFS ####################
private:
  typedef boost::shared_ptr<tvgutil::CircularQueue<std::pair<cv::Mat3b,cv::Size> > > CQ_Ptr;

  //#################### PRIVATE VARIABLES ####################
private:
  CQ_Ptr m_cq;
  std::vector<std::string> m_imagePaths;
  size_t m_imageWidthNetwork;
  size_t m_imageHeightNetwork;

  //#################### CONSTRUCTORS ####################
public:
  InputDataAssembler(const std::vector<std::string>& imagePaths, CQ_Ptr& cq, size_t imageWidthNetwork, size_t imageHeightNetwork);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  void run_load_loop();

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  std::pair<cv::Mat3b,cv::Size> read_and_resize(const std::string& path);
};

#endif
