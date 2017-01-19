/**
 * vanilla: VOCDatasetDetection.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_VOCDATASETDETECTION
#define H_VANILLA_VOCDATASETDETECTION

#include "VOCDataset.h"
#include "VOCAnnotation.h"

#include <boost/optional.hpp>

class VOCDatasetDetection : public VOCDataset
{
  //#################### CONSTRUCTORS #################### 
public:
  explicit VOCDatasetDetection(const std::string& rootDir);

  //#################### PUBLIC MEMBER FUNCTIONS #################### 
public:
  /** Override. */
  virtual std::vector<std::string> get_image_paths(VOCYear year, VOCSplit split, VOCImageType imageType, const boost::optional<size_t>& maxPathCount = boost::none, std::vector<std::string> categories = std::vector<std::string>()) const;

  /** Override. */
  virtual void visualise_annotation(const std::string& imagePath) const;

  /** Override. */
  virtual std::string get_competition_code(VOCSplit vocSplit, bool seenNonVOCData) const;

  friend std::ostream& operator<<(std::ostream& os, const VOCDatasetDetection& d);

  //#################### PRIVATE MEMBER FUNCTIONS #################### 
private:
  /** Override. */
  void initialise_annotation();
};

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const VOCDatasetDetection& d);

//#################### TYPEDEFS ####################

typedef boost::shared_ptr<VOCDatasetDetection> VOCDatasetDetection_Ptr;
typedef boost::shared_ptr<const VOCDatasetDetection> VOCDatasetDetection_CPtr;

#endif
