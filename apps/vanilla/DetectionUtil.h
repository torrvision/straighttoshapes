/**
 * vanilla: DetectionUtil.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2015, All rights reserved.
 */

#ifndef H_VANILLA_DETECTIONUTIL
#define H_VANILLA_DETECTIONUTIL

#include "core/Detection.h"
#include "core/DetectionSettings.h"

#include "dataset/VOCDetectionAnnotation.h"

#include <fstream>

#include <opencv2/core/core.hpp>

#include <darknet/network.h>

#include <tvgshape/ShapeDescriptorCalculator.h>

/**
 * \brief This struct provides utility functions.
 */
struct DetectionUtil
{
//#################### PUBLIC STATIC MEMBER FUNCTIONS ####################

static std::vector<Detections> detect(network& net, const std::vector<std::string>& paths, const DetectionSettings& ds, const boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator = boost::none);

static std::vector<Detections> detect_fast(network& net, const std::vector<std::string>& paths, const DetectionSettings& ds, const boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator = boost::none);

static Detections detect(network& net, const std::string& path, const DetectionSettings& ds, const boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator = boost::none);

static Detections detect(network& net, const cv::Mat3b& image, int originalImageWidth, int originalImageHeight, const DetectionSettings& ds, const boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator = boost::none);

static std::vector<float> get_raw_predictions(network& net, const cv::Mat3b& image, int originalImageWidth, int originalImageHeight);

static Detections extract_detections(const std::vector<float>& predictions, const DetectionSettings& ds, size_t inputImageWidth, size_t inputImageHeight, const boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator = boost::none);

static Detections non_maximal_suppression(const Detections& d, float overlapThreshold);

static Detections prune_detections(const Detections& d, float detectionThreshold);

static cv::Mat3b overlay_detections(const cv::Mat3b& im, const Detections& detections, float displayDetectionThreshold, const std::vector<std::string>& categoryNames, const std::map<size_t,cv::Scalar>& palette);

static Detections voc_objects_to_detections(const std::vector<VOCObject>& objects, size_t categoryCount);
};

#endif
