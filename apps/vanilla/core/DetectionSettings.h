/**
 * vanilla: DetectionSettings.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2015. All rights reserved.
 */

#ifndef H_VANILLA_DETECTIONSETTINGS
#define H_VANILLA_DETECTIONSETTINGS

#include <ostream>

struct DetectionSettings
{
  //#################### MEMBER VARIABLES ####################

  size_t categoryCount;
  size_t boxesPerCell;
  float detectionThreshold;
  std::string encoding;
  size_t gridSideLength;
  bool nms;
  size_t paramsPerBox;
  size_t paramsPerConfidenceScore;
  size_t paramsPerShapeEncoding;
  bool onlyObjectness;
  float overlapThreshold;
  float shapeScale;
  bool useSquare;

  //#################### CONSTRUCTORS ####################
  DetectionSettings(
    size_t categoryCount_,
    size_t boxesPerCell_,
    float detectionThreshold_,
    std::string encoding_,
    size_t gridSideLength_,
    bool nms_,
    size_t paramsPerBox_,
    size_t paramsPerConfidenceScore_,
    size_t paramsPerShapeEncoding_,
    bool onlyObjectness_ = false,
    float overlapThreshold_ = 0.5f,
    float shapeScale = 0.1f,
    bool useSquare_ = true
    );
};

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const DetectionSettings& ds);

#endif
