/**
 * vanilla: VOCDatasetDetection.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "VOCDatasetDetection.h"

#include "../DetectionUtil.h"
#include "../Util.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <boost/filesystem.hpp>

#include <tvgutil/filesystem/FilesystemUtil.h>
#include <tvgutil/containers/MapUtil.h>
using namespace tvgutil;

//#################### CONSTRUCTORS ####################

VOCDatasetDetection::VOCDatasetDetection(const std::string& rootDir)
: VOCDataset(rootDir)
{
  std::list<std::string> expectedPaths = get_split_files(VOC_ALLYEARS, VOC_DETECTION, VOC_ALLSPLITS);
  std::list<std::string> expectedCategoryPaths = get_split_files(VOC_ALLYEARS, VOC_DETECTION, VOC_ALLSPLITS, m_categories);
  std::list<std::string> missingPaths;
  missingPaths.splice(missingPaths.end(), FilesystemUtil::get_missing_paths(expectedPaths));
  missingPaths.splice(missingPaths.end(), FilesystemUtil::get_missing_paths(expectedCategoryPaths));
  output_missing_paths(missingPaths);

  initialise_annotation();
}

//#################### PUBLIC MEMBER FUNCTIONS #################### 

std::string VOCDatasetDetection::get_competition_code(VOCSplit vocSplit, bool seenNonVOCData) const
{
  std::string compCode("compX_det_" + get_split_name(vocSplit));
  if(seenNonVOCData) compCode[4] = '4';
  else compCode[4] = '3';

  return compCode;
}

std::vector<std::string> VOCDatasetDetection::get_image_paths(VOCYear year, VOCSplit split, VOCImageType imageType, const boost::optional<size_t>& maxPathCount, std::vector<std::string> categories) const
{
  std::list<std::string> splitFiles = get_split_files(year, VOC_DETECTION, split);
  std::vector<std::string> imagePaths = construct_image_paths_from_split_files(splitFiles, imageType);
  if(maxPathCount) truncate_image_paths(imagePaths, *maxPathCount);
  return imagePaths;
}

void VOCDatasetDetection::visualise_annotation(const std::string& imagePath) const
{
  cv::Mat3b im = cv::imread(imagePath, CV_LOAD_IMAGE_COLOR);
  std::string imageName = (boost::filesystem::path(imagePath)).stem().string();

  std::string annotationWindowName = imageName + " box annotation";
  // FIXME: annotation is not used except for checking
  boost::optional<VOCAnnotation_CPtr> annotation = optionally_get_annotation_from_name(imageName);
  if(annotation)
  {
    const size_t counter = 2;
    Detections groundTruth = get_detections_from_image_path(imagePath);
    cv::namedWindow(annotationWindowName, cv::WINDOW_NORMAL);
    cv::moveWindow(annotationWindowName, counter*im.cols, 0);
    cv::Mat3b annotationDisplayImage = DetectionUtil::overlay_detections(im, groundTruth, 1, get_category_names(), get_palette());
    cv::imshow(annotationWindowName, annotationDisplayImage);
  }
}

//#################### PRIVATE MEMBER FUNCTIONS #################### 

void VOCDatasetDetection::initialise_annotation()
{
  std::vector<std::string> imagePaths = get_image_paths(VOC_ALLYEARS, VOC_TRAINVAL, VOC_JPEG);
  for(size_t i = 0, size = imagePaths.size(); i < size; ++i)
  {
    boost::filesystem::path imagePath(imagePaths[i]);
    std::string imageName = imagePath.stem().string();
    std::string annotationPath = imagePath.parent_path().parent_path().string() + "/Annotations/" + imageName + ".xml";
    VOCAnnotation_Ptr annotation(new VOCDetectionAnnotation(annotationPath));
    m_imageNameToAnnotation.insert(std::make_pair(imageName, annotation));
  }
}

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const VOCDatasetDetection& d)
{
  os << "Detection Dataset\n";
  return os;
}
