/**
 * vanilla: Capture.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016, All rights reserved.
 */

#include "Capture.h"

#include <iostream>

#include <boost/lexical_cast.hpp>

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

//#################### CONSTRUCTORS ####################

Capture::Capture(const Settings& settings)
: m_settings(settings)
{
  if(m_settings.videoFile.empty())
  {
    m_cvcap.open(m_settings.webcamId);
  }
  else
  {
    m_cvcap.open(m_settings.videoFile);
  }

  if(!m_cvcap.isOpened()) throw std::runtime_error("Capture was not initialised properly");

  // Try to set the capture properties.
  m_cvcap.set(CV_CAP_PROP_FRAME_WIDTH, m_settings.webcamImageWidth);
  m_cvcap.set(CV_CAP_PROP_FRAME_HEIGHT, m_settings.webcamImageHeight);
}

//#################### PUBLIC MEMBER FUNCTIONS ####################

bool Capture::get_next_frame()
{
  if(!m_cvcap.read(frame))
  {
    std::cerr << "Failed to get a video frame";
    return false;
  }

  return true;
}

int Capture::get_image_height() const
{
  return m_cvcap.get(CV_CAP_PROP_FRAME_HEIGHT);
}

int Capture::get_image_width() const
{
  return m_cvcap.get(CV_CAP_PROP_FRAME_WIDTH);
}

std::string Capture::get_source_name() const
{
  if(m_settings.videoFile.empty())
  {
    return "webcam" + boost::lexical_cast<std::string>(m_settings.webcamId);
  }

  return (path(m_settings.videoFile)).stem().string();
}
