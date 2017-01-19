/**
 * vanilla: Tester.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016, All rights reserved.
 */

#include "Tester.h"

#include "DetectionUtil.h"

#include "core/Detection.h"

#include "dataset/VOCDatasetUtil.h"

#include <tvgplot/PaletteGenerator.h>

#include <boost/filesystem.hpp> 

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace tvgshape;

void Tester::visualise_detections(network& net, const std::vector<std::string>& imagePaths, const Dataset_CPtr& dataset, const DetectionSettings& ds, const boost::optional<ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator)
{
  int key;
  
  const std::map<size_t,cv::Scalar>& palette = tvgplot::PaletteGenerator::generate_voc_palette();

  // If no dataset is specified, assume PASCAL.
  std::vector<std::string> categoryNames = VOCDatasetUtil::get_pascal_categories();
  if(dataset) categoryNames = dataset->get_category_names();

  // Loop over all paths and display detections.
  for(size_t i = 0, size = imagePaths.size(); i < size; ++i)
  {
    cv::Mat3b im = cv::imread(imagePaths[i], CV_LOAD_IMAGE_COLOR);
    Detections detections = DetectionUtil::detect(net, im, im.cols, im.rows, ds, shapeDescriptorCalculator);

    std::string imageName = (boost::filesystem::path(imagePaths[i])).stem().string();

    std::string detectionWindowName = imageName + " overlayed detections";
    cv::namedWindow(detectionWindowName, cv::WINDOW_NORMAL);
    cv::moveWindow(detectionWindowName, 0, 0);
    cv::Mat3b detectionDisplayImage = DetectionUtil::overlay_detections(im, detections, ds.detectionThreshold, categoryNames, palette);
    cv::imshow(detectionWindowName, detectionDisplayImage);

    std::string detectionMaskWindowName = imageName + " only detections";
    cv::namedWindow(detectionMaskWindowName, cv::WINDOW_NORMAL);
    cv::moveWindow(detectionMaskWindowName, im.cols, 0);
    cv::Mat3b detectionMaskDisplayImage = cv::Mat::zeros(im.size(), CV_8UC3);
    detectionMaskDisplayImage = DetectionUtil::overlay_detections(detectionMaskDisplayImage, detections, ds.detectionThreshold, categoryNames, palette);
    cv::imshow(detectionMaskWindowName, detectionMaskDisplayImage);

    // This will not display anything if the annotation is not found.
    if(dataset) dataset->visualise_annotation(imagePaths[i]);

    key = cv::waitKey();
    if(key == 'q') break;

    cv::destroyAllWindows();
  }
}
