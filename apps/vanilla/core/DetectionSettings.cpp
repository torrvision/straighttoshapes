/**
 * vanilla: DetectionSettings.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2015, All rights reserved.
 */

#include "DetectionSettings.h"

//#################### CONSTRUCTORS ####################

DetectionSettings::DetectionSettings(
  size_t categoryCount_,
  size_t boxesPerCell_,
  float detectionThreshold_,
  std::string encoding_,
  size_t gridSideLength_,
  bool nms_,
  size_t paramsPerBox_,
  size_t paramsPerConfidenceScore_,
  size_t paramsPerShapeEncoding_,
  bool onlyObjectness_,
  float overlapThreshold_,
  float shapeScale_,
  bool useSquare_
  )
: categoryCount(categoryCount_),
  boxesPerCell(boxesPerCell_),
  detectionThreshold(detectionThreshold_),
  encoding(encoding_),
  gridSideLength(gridSideLength_),
  nms(nms_),
  paramsPerBox(paramsPerBox_),
  paramsPerConfidenceScore(paramsPerConfidenceScore_),
  paramsPerShapeEncoding(paramsPerShapeEncoding_),
  onlyObjectness(onlyObjectness_),
  overlapThreshold(overlapThreshold_),
  shapeScale(shapeScale_),
  useSquare(useSquare_)
{}

//#################### OUTPUT ####################

#define PRT(X) os << #X << ": " << X << '\n'
std::ostream& operator<<(std::ostream& os, const DetectionSettings& ds)
{
  PRT(ds.categoryCount);
  PRT(ds.boxesPerCell);
  PRT(ds.detectionThreshold);
  PRT(ds.encoding);
  PRT(ds.gridSideLength);
  PRT(ds.nms);
  PRT(ds.paramsPerBox);
  PRT(ds.paramsPerConfidenceScore);
  PRT(ds.paramsPerShapeEncoding);
  PRT(ds.onlyObjectness);
  PRT(ds.overlapThreshold);
  PRT(ds.shapeScale);
  PRT(ds.useSquare);
  return os;
}
#undef PRT
