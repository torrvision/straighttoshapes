/**
 * vanilla: Tester.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016, All rights reserved.
 */

#ifndef H_VANILLA_TESTER
#define H_VANILLA_TESTER

#include "core/DetectionSettings.h"

#include "dataset/Dataset.h"

#include <darknet/network.h>

#include <vector>
#include <string>

#include <tvgshape/ShapeDescriptorCalculator.h>

/**
 * \brief TODO.
 */
class Tester
{
//#################### PUBLIC STATIC MEMBER FUNCTIONS ####################
public:
  static void visualise_detections(network& net, const std::vector<std::string>& imagePaths, const Dataset_CPtr& dataset, const DetectionSettings& ds, const boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator = boost::none);
};

#endif
