/**
 * vanilla: Demo.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "Demo.h"
#include "Util.h"
#include "DetectionUtil.h"

#include "core/Detection.h"
#include "core/MovingAverage.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <tvgutil/numbers/NumberSequenceGenerator.h>
#include <tvgutil/timing/Timer.h>
#include <tvgutil/timing/TimeUtil.h>
using namespace tvgutil;
using namespace tvgshape;

#include <tvgplot/PaletteGenerator.h>
using namespace tvgplot;

//#################### CONSTRUCTORS ####################

Demo::Demo(const Demo::Settings& demoSettings, const Capture::Settings& capSettings, const DetectionSettings& ds)
: m_capture(capSettings),
  m_createdWindow(false),
  m_demoSettings(demoSettings),
  m_detectionSettings(ds),
  m_detectionThreshold(0.2 * 10000),
  m_frameNumber(0),
  m_overlapThreshold(ds.overlapThreshold * 10),
  m_movingPredictionAverage(3, std::vector<float>(ds.gridSideLength * ds.gridSideLength
                    * ((ds.paramsPerConfidenceScore + ds.paramsPerBox + ds.paramsPerShapeEncoding)*ds.boxesPerCell + ds.categoryCount),0))
{
  std::vector<size_t> categoryIds = NumberSequenceGenerator::generate_stepped<size_t>(0, 1, 20);
  m_demoPalette = PaletteGenerator::generate_random_rgba_palette(std::set<size_t>(categoryIds.begin(), categoryIds.end()), 1234);
}

//#################### PUBLIC MEMBER FUNCTIONS ####################

void Demo::run(network& net, const std::vector<std::string>& datasetCategoryNames, const boost::optional<ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator)
{
  while(1)
  {
    ++m_frameNumber;

TIME( // Capture a video frame.
    bool captureSuccess = m_capture.get_next_frame();

, milliseconds, capture); std::cout << capture << '\n';

    if(!captureSuccess) break;
    else
    {
      const cv::Mat3b& im = m_capture.frame;

TIME( // Run the detection.
    std::vector<float> predictions = m_movingPredictionAverage.push(DetectionUtil::get_raw_predictions(net, im, im.cols, im.rows));

    Detections detections = DetectionUtil::extract_detections(predictions, m_detectionSettings, im.cols, im.rows, shapeDescriptorCalculator);

    if(m_detectionSettings.nms)
    {
      detections = DetectionUtil::non_maximal_suppression(detections, m_detectionSettings.overlapThreshold);
    }

, microseconds, detectionStep);
    std::cout << "detectionStep:" << detectionStep.duration().count()/1000.0 << '\n';
    static MovingAverage<double> processingTimeAverage(50, 0.0);
    m_processingTime = processingTimeAverage.push(double(detectionStep.duration().count()/1000.0));

TIME( // Overlay detecitons and convert back to OpenCV image.
    cv::Mat3b displayImage = DetectionUtil::overlay_detections(im, detections, m_detectionSettings.detectionThreshold, datasetCategoryNames, m_demoPalette);
    draw_info(displayImage);
    display_image(displayImage);

, milliseconds, overlayAndDisplay); std::cout << overlayAndDisplay << '\n';

    if(!m_demoSettings.saveDir.empty()) save_image(displayImage);

TIME(
    if(terminate()) break;

, milliseconds, cleanAndTerminate); std::cout << cleanAndTerminate << '\n' << std::endl;
    }
  }
}

void Demo::save_image(const cv::Mat3b& image)
{
  static std::string fileDir = m_demoSettings.saveDir + '/' + m_capture.get_source_name() + TimeUtil::get_iso_timestamp();

  if(!boost::filesystem::exists(fileDir)) boost::filesystem::create_directories(fileDir);

  boost::format sixDigits("%06d");
  std::string fileName = "image" + (sixDigits % m_frameNumber).str() + ".png";
  std::string filePath = fileDir + '/' + fileName;
  cv::imwrite(filePath, image, boost::assign::list_of(0));
}

void Demo::display_image(const cv::Mat3b& displayImage)
{
  const std::string windowName("Demo");
  int xGridId(1);
  int yGridId(1);

  if(!m_createdWindow)
  {
    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::moveWindow(windowName, xGridId*m_capture.get_image_width(), yGridId*m_capture.get_image_height());

    if(m_demoSettings.debugFlag)
    {
      cv::createTrackbar("delay", windowName, &m_demoSettings.debugWaitTimeMs, 3000);

      // Create an integer detection threshold which can be varied form 0.0001 to 0.5.
      cv::createTrackbar("detectionThreshold*10000", windowName, &m_detectionThreshold, 5000);
      cv::createTrackbar("overlapThreshold*10", windowName, &m_overlapThreshold, 10);
    }

    m_createdWindow = true;
  }

  if(m_demoSettings.debugFlag)
  {
    m_detectionThreshold = m_detectionThreshold > 0 ? m_detectionThreshold : 1;
    m_detectionSettings.detectionThreshold = static_cast<float>(m_detectionThreshold / 10000.0f);

    m_overlapThreshold = m_overlapThreshold > 0 ? m_overlapThreshold : 1;
    m_detectionSettings.overlapThreshold = static_cast<float>(m_overlapThreshold / 10.0f);
  }

  cv::imshow(windowName, displayImage);
}

bool Demo::terminate()
{
  int inputKey;
  if(m_demoSettings.debugFlag) inputKey = cv::waitKey(m_demoSettings.debugWaitTimeMs);
  else                         inputKey = cv::waitKey(m_demoSettings.standardWaitTimeMs);

  if(inputKey == 'p') cv::waitKey(0); // Pause.

  if(inputKey == 'q' || inputKey == 1048689) return true;

  return false;
}

void Demo::draw_info(cv::Mat& im)
{
  boost::format twoDP("%0.2f");
  boost::format oneDP("%0.1f");
  std::string processingTime = (twoDP % m_processingTime).str();
  std::string fps = (oneDP % (1000.0 / m_processingTime)).str();
  std::string text = processingTime + "ms / " + fps + "fps";
  cv::Point2i pos(im.cols - 270, im.rows - 16);
  putText(im, text, pos, cv::FONT_HERSHEY_SIMPLEX, 0.8, CV_RGB(0,200,0), 2);
}
