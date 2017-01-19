/**
 * vanilla: VOCDetectionAnnotation.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2015. All rights reserved.
 */

#ifndef H_VANILLA_VOCDETECTIONANNOTATION
#define H_VANILLA_VOCDETECTIONANNOTATION

#include "VOCAnnotation.h"
#include "../core/VOCObject.h"

class VOCDetectionAnnotation : public VOCAnnotation
{
  //#################### PRIVATE MEMBER VARIABLES ####################
private:
  std::vector<VOCObject> m_objects;

  //#################### CONSTRUCTORS #################### 
public:
  VOCDetectionAnnotation(const std::string& annotationPath);

  //#################### PUBLIC MEMBER FUNCTIONS #################### 
public:
  /** Override. */
  virtual std::vector<VOCObject> get_objects(const boost::optional<DataTransformation>& dataTransformation = boost::none) const;

  /** Override. */
  //virtual void save(const std::string& saveDir) const;

  //#################### PRIVATE MEMBER FUNCTIONS #################### 
private:
  /** Override. */
  virtual void read_annotation(const std::string& path);
};

//#################### OUTPUT #################### 

std::ostream& operator<<(std::ostream& os, const VOCDetectionAnnotation& annotation);

//#################### TYPEDEFS ####################

typedef boost::shared_ptr<VOCDetectionAnnotation> VOCDetectionAnnotation_Ptr;
typedef boost::shared_ptr<const VOCDetectionAnnotation> VOCDetectionAnnotation_CPtr;

#endif
