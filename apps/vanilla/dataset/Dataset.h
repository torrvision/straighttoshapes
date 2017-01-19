/**
 * vanilla: Dataset.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2015. All rights reserved.
 */

#ifndef H_VANILLA_DATASET
#define H_VANILLA_DATASET

#include <iostream>
#include <list>
#include <map>
#include <vector>

#include <opencv2/core/core.hpp>

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include "../Util.h"
#include "../core/Detection.h"
#include "../data/DataTransformation.h"

#include "VOCAnnotation.h"

enum VOCTask
{
  VOC_SBD,
  VOC_SEGMENTATION,
  VOC_DETECTION
};

enum VOCSplit
{
  VOC_TRAIN,
  VOC_TRAINVAL,
  VOC_VAL,
  VOC_TEST,
  VOC_ALLSPLITS
};

enum VOCYear
{
  VOC_2007,
  VOC_2012,
  COCO_2014,
  VOC_ALLYEARS,
  COCO_ALLYEARS
};

enum VOCImageType
{
  VOC_JPEG,
  VOC_SEMANTIC,
  VOC_INSTANCE
};

class Dataset
{
  //#################### PROTECTED MEMBER VARIABLES ####################
protected:
  /** The development kit root directory. */
  std::string m_rootDir;

  /** The categories in the dataset. */
  std::vector<std::string> m_categories;
  std::map<std::string,size_t> m_categoryNameToId;
  std::map<size_t,std::string> m_categoryIdToName;

  /** The types of splits. */
  std::vector<std::string> m_splitNames;

  std::vector<std::string> m_vocYears;

  std::vector<std::string> m_vocTasks;

  std::vector<std::string> m_imageTypeNames;

  std::map<std::string, std::string> m_imageTypeExtension;

  // Colour maps and palettes.
  std::map<size_t,cv::Scalar> m_vocPalette;
  std::map<size_t,cv::Vec3b> m_categoryIdToColour;
  boost::unordered_map<cv::Vec3b,size_t,Vec3bHash> m_colourToCategoryIdHash;

  std::map<std::string,VOCAnnotation_CPtr> m_imageNameToAnnotation;

  //#################### CONSTRUCTORS ####################
public:
  explicit Dataset(const std::string& rootDir);

  //#################### DESTRUCTORS ####################
public:
  virtual ~Dataset();

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  virtual std::vector<std::string> get_image_paths(VOCYear year, VOCSplit split, VOCImageType imageType, const boost::optional<size_t>& maxPathCount = boost::none, std::vector<std::string> categories = std::vector<std::string>()) const = 0;

  virtual void visualise_annotation(const std::string& imageName) const = 0;

  virtual std::string get_competition_code(VOCSplit vocSplit, bool seenNonVOCData) const = 0;

  VOCAnnotation_CPtr get_annotation_from_image_path(const std::string& path) const;
  VOCAnnotation_CPtr get_annotation_from_image_name(const std::string& imageName) const;
  boost::optional<VOCAnnotation_CPtr> optionally_get_annotation_from_name(const std::string& name) const;

  Detections get_detections_from_image_path(const std::string& imagePath, const boost::optional<DataTransformation>& dataTransformation = boost::none) const;
  Detections get_detections_from_image_name(const std::string& imageName, const boost::optional<DataTransformation>& dataTransformation = boost::none) const;

  std::map<std::string,size_t> get_category_name_to_id() const;
  std::map<size_t,std::string> get_category_id_to_name() const;
  std::vector<std::string> get_category_names() const;
  std::string get_split_name(VOCSplit vocSplit) const;
  std::string get_year_name(VOCYear vocYear) const;

  //std::string get_save_model_path() const;
  std::string get_dir_in_results(const std::string& subDir) const;

  std::map<size_t,cv::Vec3b> get_category_id_to_colour() const;
  boost::unordered_map<cv::Vec3b,size_t,Vec3bHash> get_colour_to_category_id_hash() const;
  std::map<size_t,cv::Scalar> get_palette() const;

  friend std::ostream& operator<<(std::ostream& os, const Dataset& d);

  //#################### PROTECTED MEMBER FUNCTIONS ####################
protected:
  virtual void initialise_annotation() = 0;

  void output_missing_paths(std::list<std::string>& missingPaths) const;

  void truncate_image_paths(std::vector<std::string>& imagePaths, size_t maximumSize) const;
};

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const Dataset& d);

//#################### TYPEDEFS ####################

typedef boost::shared_ptr<Dataset> Dataset_Ptr;
typedef boost::shared_ptr<const Dataset> Dataset_CPtr;

#endif
