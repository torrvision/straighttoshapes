/**
 * vanilla: Capture.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016, All rights reserved.
 */

#ifndef H_VANILLA_CAPTURE
#define H_VANILLA_CAPTURE

#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

/*
 * \brief TODO.
 */
class Capture
{
  //#################### NESTED TYPES ####################
public:
  struct Settings
  {
    //~~~~~~~~~~~~~~~~~~~~ PUBLIC VARIABLES ~~~~~~~~~~~~~~~~~~~~
    //CaptureType captureType;
    std::string videoFile;
    int webcamId;
    int webcamImageHeight;
    int webcamImageWidth;
  };

  /** The current frame. */
  cv::Mat3b frame;

  //#################### PRIVATE VARIABLES ####################
private:
  /** The capture settings. */
  Settings m_settings;

  /** The opencv capture object. */
  cv::VideoCapture m_cvcap;

  //#################### CONSTRUCTORS ####################
public:
  Capture(const Capture::Settings& settings);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  bool get_next_frame();
  int get_image_height() const;
  int get_image_width() const;
  std::string get_source_name() const;
};

#endif
