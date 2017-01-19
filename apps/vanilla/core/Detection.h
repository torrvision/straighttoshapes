/**
 * vanilla: Detection.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_DETECTION
#define H_VANILLA_DETECTION

#include "Shape.h"

#include <iostream>

#include <vector>

#include <boost/tuple/tuple.hpp>

//#################### TYPEDEFS ####################

typedef boost::tuple<std::string, Shape, float> NamedCategoryDetection;
typedef std::vector<NamedCategoryDetection> NamedCategoryDetections;

typedef std::pair<Shape, std::vector<float> > Detection;
typedef std::vector<Detection> Detections;

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const Detections& d);
std::ostream& operator<<(std::ostream& os, const NamedCategoryDetections& d);

#endif
