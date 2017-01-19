/**
 * vanilla: Dataset.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2015. All rights reserved.
 */

#include "Dataset.h"
#include "VOCAnnotation.h"

#include "../DetectionUtil.h"

#include <fstream>
#include <stdexcept>

#include <boost/assign/list_of.hpp>
#include <boost/filesystem.hpp>

#include <tvgutil/persistence/LineUtil.h>
#include <tvgutil/filesystem/FilesystemUtil.h>
#include <tvgutil/containers/MapUtil.h>
using namespace tvgutil;

#include <tvgplot/PaletteGenerator.h>
using namespace tvgplot;

//#################### CONSTRUCTORS ####################

Dataset::Dataset(const std::string& rootDir)
: m_rootDir(rootDir)
{
  m_vocYears = boost::assign::list_of<std::string>("2007")("2012")("2014").to_container(m_vocYears);

  m_vocTasks = boost::assign::list_of<std::string>("SBD")("Segmentation")("Main").to_container(m_vocTasks);

  m_splitNames = boost::assign::list_of<std::string>("train")("trainval")("val")("test").to_container(m_splitNames);

  m_imageTypeNames = boost::assign::list_of<std::string>("JPEGImages")("SegmentationClass")("SegmentationObject")("SBDSegmentationClass")("SBDSegmentationObject").to_container(m_imageTypeNames);

  m_imageTypeExtension = boost::assign::map_list_of(m_imageTypeNames[VOC_JPEG], ".jpg")
                                                   (m_imageTypeNames[VOC_SEMANTIC], ".png")
                                                   (m_imageTypeNames[VOC_INSTANCE], ".png")
                                                   .to_container(m_imageTypeExtension);

  m_vocPalette = PaletteGenerator::generate_voc_palette();
  std::map<size_t,cv::Scalar>::iterator it, iend;
  for(it = m_vocPalette.begin(); it != m_vocPalette.end(); ++it)
  {
    size_t categoryId = it->first;
    cv::Vec3b colour(static_cast<uint8_t>(it->second.val[0]),
                     static_cast<uint8_t>(it->second.val[1]),
                     static_cast<uint8_t>(it->second.val[2]));
    m_categoryIdToColour.insert(std::make_pair(categoryId,colour));
    m_colourToCategoryIdHash.insert(std::make_pair(colour,categoryId));
  }

  // Initialise the static members of the voc annotation.
  VOCAnnotation::set_colour_to_category_id_hash(m_colourToCategoryIdHash);
}

//#################### DESTRUCTORS ####################

Dataset::~Dataset(){}

//#################### PUBLIC MEMBER FUNCTIONS ####################

Detections Dataset::get_detections_from_image_path(const std::string& imagePath, const boost::optional<DataTransformation>& dataTransformation) const
{
  boost::filesystem::path bpath(imagePath);
  return get_detections_from_image_name(bpath.stem().string(), dataTransformation);
}

Detections Dataset::get_detections_from_image_name(const std::string& imageName, const boost::optional<DataTransformation>& dataTransformation) const
{
  VOCAnnotation_CPtr annotation = get_annotation_from_image_path(imageName);
  const std::vector<VOCObject>& objects = annotation->get_objects(dataTransformation);
  return DetectionUtil::voc_objects_to_detections(objects, VOCDetectionAnnotation::get_category_count());
}

VOCAnnotation_CPtr Dataset::get_annotation_from_image_path(const std::string& path) const
{
  boost::filesystem::path bpath(path);
  return get_annotation_from_image_name(bpath.stem().string());
}

VOCAnnotation_CPtr Dataset::get_annotation_from_image_name(const std::string& name) const
{
  VOCAnnotation_CPtr annotation = MapUtil::lookup(m_imageNameToAnnotation, name);
  return annotation;
}

boost::optional<VOCAnnotation_CPtr> Dataset::optionally_get_annotation_from_name(const std::string& name) const
{
  std::map<std::string,VOCAnnotation_CPtr>::const_iterator it = m_imageNameToAnnotation.find(name);
  if(it != m_imageNameToAnnotation.end()) return it->second;
  else return boost::none;
}

std::map<size_t,cv::Vec3b> Dataset::get_category_id_to_colour() const
{
  return m_categoryIdToColour;
}

std::map<std::string,size_t> Dataset::get_category_name_to_id() const
{
  return m_categoryNameToId;
}

std::map<size_t,std::string> Dataset::get_category_id_to_name() const
{
  return m_categoryIdToName;
}

std::vector<std::string> Dataset::get_category_names() const
{
  return m_categories;
}

boost::unordered_map<cv::Vec3b,size_t,Vec3bHash> Dataset::get_colour_to_category_id_hash() const
{
  return m_colourToCategoryIdHash;
}

std::map<size_t,cv::Scalar> Dataset::get_palette() const
{
  return m_vocPalette;
}

std::string Dataset::get_split_name(VOCSplit vocSplit) const
{
  return m_splitNames[vocSplit];
}

std::string Dataset::get_year_name(VOCYear vocYear) const
{
  return m_vocYears[vocYear];
}

/*
std::string Dataset::get_save_model_path() const
{
  boost::filesystem::path root(m_rootDir);
  std::string saveModelPath = root.parent_path().parent_path().string();
  return saveModelPath + "/models";
}
*/

std::string Dataset::get_dir_in_results(const std::string& subDir) const
{
  boost::filesystem::path root(m_rootDir);
  std::string path = root.parent_path().parent_path().string()
                                + "/results/detection/" + subDir;

  if(!boost::filesystem::exists(path)) boost::filesystem::create_directories(path);
  return path;
}

//#################### PROTECTED MEMBER FUNCTIONS ####################

void Dataset::output_missing_paths(std::list<std::string>& missingPaths) const
{
  if(!missingPaths.empty())
  {
    std::cout << "[vocdataset] Expecting to see: \n";
    for(std::list<std::string>::iterator it = missingPaths.begin(), iend = missingPaths.end(); it != iend; ++it)
    {
      std::cout << *it << '\n';
    }

    throw std::runtime_error("Error: The aforementioned files and directories were not found, please make sure you have the correct dataset");
  }
}

void Dataset::truncate_image_paths(std::vector<std::string>& imagePaths, size_t maximumSize) const
{
  if(maximumSize <= imagePaths.size())
  {
    imagePaths.erase(imagePaths.begin() + maximumSize, imagePaths.end());
  }
}

//#################### OUTPUT ####################

#define PRT(X) os << #X << ": " << X << '\n'
std::ostream& operator<<(std::ostream& os, const Dataset& d)
{
  os << "PASCAL VOC Dataset\n";
  PRT(d.m_rootDir);
  return os;
}
#undef PRT
