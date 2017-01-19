/**
 * vanilla: VOCDatasetSBD.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_VOCDATASETSBD
#define H_VANILLA_VOCDATASETSBD

#include "VOCDataset.h"
#include "VOCAnnotation.h"

#include <boost/optional.hpp>

class VOCDatasetSBD : public VOCDataset
{
  //#################### CONSTRUCTORS #################### 
public:
  explicit VOCDatasetSBD(const std::string& rootDir);

  //#################### PUBLIC MEMBER FUNCTIONS #################### 
public:
  /** Override. */
  virtual std::vector<std::string> get_image_paths(VOCYear year, VOCSplit split, VOCImageType imageType, const boost::optional<size_t>& maxPathCount = boost::none, std::vector<std::string> categories = std::vector<std::string>()) const;

  /** Override. */
  virtual void visualise_annotation(const std::string& imagePath) const;

  /** Override. */
  virtual std::string get_competition_code(VOCSplit vocSplit, bool seenNonVOCData) const;

  friend std::ostream& operator<<(std::ostream& os, const VOCDatasetSBD& d);

  //#################### PRIVATE MEMBER FUNCTIONS #################### 
private:
  /** Override. */
  void initialise_annotation();
};

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const VOCDatasetSBD& d);

//#################### TYPEDEFS ####################

typedef boost::shared_ptr<VOCDatasetSBD> VOCDatasetSBD_Ptr;
typedef boost::shared_ptr<const VOCDatasetSBD> VOCDatasetSBD_CPtr;

#endif
