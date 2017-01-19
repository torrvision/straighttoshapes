/**
 * vanilla: COCODatasetInstance.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_COCODATASETINSTANCE
#define H_VANILLA_COCODATASETINSTANCE

#include "Dataset.h"
#include "VOCAnnotation.h"

#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/unordered_map.hpp>

#include "COCODataPrimitives.h"

class COCODatasetInstance : public Dataset
{
  //#################### TYPEDEFS ####################
public:
  typedef std::vector<float> FloatVector;
  typedef std::vector<FloatVector> FloatVectors;
  typedef std::map<std::string,std::vector<std::string> > StringToVectorOfStrings;

  typedef boost::unordered_map<size_t, AnnotationData> AnnotationDataHash;
  typedef boost::shared_ptr<AnnotationDataHash> AnnotationDataHash_Ptr;
  typedef boost::shared_ptr<const AnnotationDataHash> AnnotationDataHash_CPtr;

  typedef boost::unordered_map<size_t, CategoryData> CategoryDataHash;
  typedef boost::shared_ptr<CategoryDataHash> CategoryDataHash_Ptr;
  typedef boost::shared_ptr<const CategoryDataHash> CategoryDataHash_CPtr;

  typedef boost::unordered_map<size_t, ImageData> ImageDataHash;
  typedef boost::shared_ptr<ImageDataHash> ImageDataHash_Ptr;
  typedef boost::shared_ptr<const ImageDataHash> ImageDataHash_CPtr;


  //#################### PRIVATE MEMBER VARIABLES ####################
private:
  StringToVectorOfStrings m_splitYearToImagePaths;
  std::map<std::string,std::vector<size_t> > m_imageNameToObjectIds;
  std::map<size_t,size_t> m_cocoCategoryIdsToCategoryIds;
  std::vector<size_t> m_cocoCategoryIds;

  AnnotationDataHash_Ptr m_annotationDataHash;
  CategoryDataHash_Ptr m_categoryDataHash;
  ImageDataHash_Ptr m_imageDataHash;

  //#################### CONSTRUCTORS ####################
public:
  explicit COCODatasetInstance(const std::string& rootDir);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  /** Override. */
  virtual std::vector<std::string> get_image_paths(VOCYear year, VOCSplit split, VOCImageType imageType, const boost::optional<size_t>& maxPathCount = boost::none, std::vector<std::string> categories = std::vector<std::string>()) const;

  /** Override. */
  virtual void visualise_annotation(const std::string& imagePath) const;

  /** Override. */
  virtual std::string get_competition_code(VOCSplit vocSplit, bool seenNonVOCData) const;

  friend std::ostream& operator<<(std::ostream& os, const COCODatasetInstance& d);

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  /** Override. */
  void initialise_annotation();

  std::string get_file_in_annotation_dir(const std::string& fileName) const;
  std::string get_annotation_file(VOCYear vocYear, VOCSplit vocSplit) const;

  void process_annotation_file(VOCYear year, VOCSplit split);
  void read_annotation_data(const boost::property_tree::ptree& tree);
  void read_category_data(const boost::property_tree::ptree& tree);
  void read_image_data(const boost::property_tree::ptree& tree, const std::string& splitYear);
  FloatVectors annotation_to_polygons(const boost::property_tree::ptree& tree);
  FloatVector extract_float_vector(const boost::property_tree::ptree& tree);
};

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const COCODatasetInstance& d);

//#################### TYPEDEFS ####################

typedef boost::shared_ptr<COCODatasetInstance> COCODatasetInstance_Ptr;
typedef boost::shared_ptr<const COCODatasetInstance> COCODatasetInstance_CPtr;

#endif
