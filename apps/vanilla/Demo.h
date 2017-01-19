/**
 * vanilla: Demo.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_APPLICATION
#define H_VANILLA_APPLICATION

#include "capture/Capture.h"

#include "core/DetectionSettings.h"
#include "core/MovingVectorAverage.h"

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

#include <darknet/network.h>

#include <map>

#include <tvgshape/ShapeDescriptorCalculator.h>


/**
 * \brief The main application class for vanilla.
 */
class Demo
{
  //#################### NESTED TYPES ####################
public:
  struct Settings
  {
    //~~~~~~~~~~~~~~~~~~~~ PUBLIC VARIABLES ~~~~~~~~~~~~~~~~~~~~
    /** A flag to indicate debug mode. */
    bool debugFlag;
    int debugWaitTimeMs;
    int standardWaitTimeMs;
    std::string saveDir;
  };

  //#################### PRIVATE VARIABLES ####################
private:
  /** An object to take care of video capture. */
  Capture m_capture;

  /** Whether or not the display window has been created.*/
  bool m_createdWindow;

  /** The colours to use for displaying distinct categories. */
  std::map<size_t,cv::Scalar> m_demoPalette;

  /** The settings to use with the demo application. */
  Settings m_demoSettings;

  DetectionSettings m_detectionSettings;

  /** An integer version of the detection threshold used with the opencv trackbar in debug mode. */
  int m_detectionThreshold;

  size_t m_frameNumber;

  /** The time to process one frame. */
  double m_processingTime;

  /** An integer sercsion of the non-maximal suppression threshold used with the opencv trackbar in debug mode. */
  int m_overlapThreshold;

  /** A circular buffer holding the last n predictions. */
  MovingVectorAverage<float> m_movingPredictionAverage;

  //#################### CONSTRUCTORS ####################
public:
  /**
   * \brief Constructs the application.
   */
  Demo(const Demo::Settings& demoSettings, const Capture::Settings& capSettings, const DetectionSettings& detectionSettings);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  /**
   * \brief Runs the application.
   */
  void run(network& net, const std::vector<std::string>& datasetCategoryNames, const boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator = boost::none);

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  void display_image(const cv::Mat3b& displayImage);
  void save_image(const cv::Mat3b& image);
  bool terminate();
  void draw_info(cv::Mat& im);
};

#endif
