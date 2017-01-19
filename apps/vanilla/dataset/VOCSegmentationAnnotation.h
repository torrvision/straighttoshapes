/**
 * vanilla: VOCSegmentationAnnotation.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2015. All rights reserved.
 */

#ifndef H_VANILLA_VOCSEGMENTATIONANNOTATION
#define H_VANILLA_VOCSEGMENTATIONANNOTATION

#include "VOCAnnotation.h"

#include <opencv2/core/core.hpp>

class VOCSegmentationAnnotation : public VOCAnnotation
{
  //#################### PUBLIC MEMBER VARIABLES ####################
public:
  std::string m_segmentationClassAnnotationPath;
  std::string m_segmentationObjectAnnotationPath;

  //#################### CONSTRUCTORS ####################
public:
  VOCSegmentationAnnotation(const std::string& segmentationClassAnnotationPath, const std::string& segmentationObjectAnnotationPath);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  cv::Mat3b load_class_annotation() const;
  cv::Mat3b load_object_annotation() const;

  /** Override. */
  virtual std::vector<VOCObject> get_objects(const boost::optional<DataTransformation>& dataTransformation = boost::none) const;

  /** Override. */
  //virtual void save(const std::string& saveDir) const;

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  /** Override. */
  void read_annotation(const std::string& path);
};

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const VOCSegmentationAnnotation& annotation);

//#################### TYPEDEFS ####################

typedef boost::shared_ptr<VOCSegmentationAnnotation> VOCSegmentationAnnotation_Ptr;
typedef boost::shared_ptr<const VOCSegmentationAnnotation> VOCSegmentationAnnotation_CPtr;

#endif
