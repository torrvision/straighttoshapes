/**
 * vanilla: COCODatasetInstance.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "../Util.h"
#include "COCODatasetInstance.h"
#include "COCOAnnotation.h"

#include "../DetectionUtil.h"

#include <boost/filesystem.hpp>
#include <boost/assign/list_of.hpp>
using namespace boost::assign;
using namespace boost::property_tree;

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <tvgutil/filesystem/FilesystemUtil.h>
#include <tvgutil/timing/Timer.h>
#include <tvgutil/persistence/PropertyUtil.h>
#include <tvgutil/containers/MapUtil.h>
#include <tvgutil/containers/LimitedContainer.h>
#include <tvgutil/persistence/SerializationUtil.h>
using namespace tvgutil;

//#################### CONSTRUCTORS ####################

COCODatasetInstance::COCODatasetInstance(const std::string& rootDir)
: Dataset(rootDir),
  m_annotationDataHash(new AnnotationDataHash),
  m_categoryDataHash(new CategoryDataHash),
  m_imageDataHash(new ImageDataHash)
{
  std::list<std::string> expectedPaths;
  const std::string annotationPath = rootDir + "/annotations";
  expectedPaths.push_back(get_annotation_file(COCO_2014, VOC_TRAIN));
  expectedPaths.push_back(get_annotation_file(COCO_2014, VOC_VAL));
  expectedPaths.push_back(rootDir + "/train2014");
  expectedPaths.push_back(rootDir + "/val2014");

  // TODO Remove this from the base class.
  //std::list<std::string> expectedPaths = get_split_files(VOC_ALLYEARS, VOC_SEGMENTATION, VOC_ALLSPLITS);
  std::list<std::string> missingPaths;
  missingPaths.splice(missingPaths.end(), FilesystemUtil::get_missing_paths(expectedPaths));
  output_missing_paths(missingPaths);

  initialise_annotation();
}

//#################### PUBLIC MEMBER FUNCTIONS ####################

std::string COCODatasetInstance::get_competition_code(VOCSplit vocSplit, bool seenNonVOCData) const
{
  return "coco_inst_" + get_split_name(vocSplit);
}

std::vector<std::string> COCODatasetInstance::get_image_paths(VOCYear year, VOCSplit split, VOCImageType imageType, const boost::optional<size_t>& maxPathCount, std::vector<std::string> categories) const
{
  // FIXME copied form VOCDataset.cpp
  std::vector<std::string> years;
  if(year == COCO_2014 || year == COCO_ALLYEARS)
  {
    years.push_back(m_vocYears[COCO_2014]);
  }

  std::vector<std::string> splits;
  if(split == VOC_TRAIN || split == VOC_VAL)
  {
    splits.push_back(m_splitNames[split]);
  }
  else if(split == VOC_ALLSPLITS || split == VOC_TRAINVAL)
  {
    splits.push_back(m_splitNames[VOC_TRAIN]);
    splits.push_back(m_splitNames[VOC_VAL]);
  }

  std::vector<std::string> imagePaths;
  for(size_t year = 0, yearCount = years.size(); year < yearCount; ++year)
  {
    for(size_t split = 0, splitCount = splits.size(); split < splitCount; ++split)
    {
      std::string splitYear = splits[split] + years[year];
      std::vector<std::string> splitPaths = MapUtil::lookup(m_splitYearToImagePaths, splitYear);
      imagePaths.insert(imagePaths.end(), splitPaths.begin(), splitPaths.end());
    }
  }

  if(maxPathCount) truncate_image_paths(imagePaths, *maxPathCount);
  return imagePaths;
}

void COCODatasetInstance::visualise_annotation(const std::string& imagePath) const
{
  cv::Mat3b im = cv::imread(imagePath, CV_LOAD_IMAGE_COLOR);
  std::string imageName = (boost::filesystem::path(imagePath)).stem().string();
  boost::optional<VOCAnnotation_CPtr> annotation = optionally_get_annotation_from_name(imageName);
  if(annotation)
  {
    const size_t counter = 2;
    Detections groundTruth = get_detections_from_image_path(imagePath);

    std::string shapeDetectionWindowName = imageName + " overlayed shape annotation";
    cv::namedWindow(shapeDetectionWindowName, cv::WINDOW_NORMAL);
    cv::moveWindow(shapeDetectionWindowName, (counter)*im.cols, 0);
    cv::Mat3b shapeDetectionDisplayImage = DetectionUtil::overlay_detections(im, groundTruth, 1, get_category_names(), get_palette());
    cv::imshow(shapeDetectionWindowName, shapeDetectionDisplayImage);

    std::string maskWindow = imageName + " shape annotation";
    cv::namedWindow(maskWindow, cv::WINDOW_NORMAL);
    cv::moveWindow(maskWindow, (counter+1)*im.cols, 0);
    cv::Mat3b maskImage = cv::Mat::zeros(im.size(), CV_8UC3);
    maskImage = DetectionUtil::overlay_detections(maskImage, groundTruth, 1, get_category_names(), get_palette());
    cv::imshow(maskWindow, maskImage);
  }
}

//#################### PRIVATE MEMBER FUNCTIONS ####################

std::string COCODatasetInstance::get_file_in_annotation_dir(const std::string& fileName) const
{
  return m_rootDir + "/annotations/" + fileName;
}

std::string COCODatasetInstance::get_annotation_file(VOCYear vocYear, VOCSplit vocSplit) const
{
  return get_file_in_annotation_dir("instances_" + get_split_name(vocSplit) + get_year_name(vocYear) + '.' + "json");
}

void COCODatasetInstance::initialise_annotation()
{
  tvgutil::Timer<boost::chrono::milliseconds> processAnnotationTime("processAnnotationTime");

  // The data structures that are populated by process_annotation_file:
  // If any of these files are missing reprocess them
  std::vector<std::string> expectedPaths  = list_of(get_file_in_annotation_dir("annotationData.vec"))
                                                   (get_file_in_annotation_dir("categoryData.vec"))
                                                   (get_file_in_annotation_dir("imageData.vec"))
                                                   (get_file_in_annotation_dir("splitYearToImagePaths.map"));

  bool missingFiles = false;
  for(size_t i = 0; i < expectedPaths.size(); ++i)
  {
    if(!boost::filesystem::exists(expectedPaths[i]))
    {
      missingFiles = true;
      break;
    }
  }

  boost::shared_ptr<std::vector<AnnotationData> > annotationDataVector(new std::vector<AnnotationData>());
  boost::shared_ptr<std::vector<CategoryData> > categoryDataVector(new std::vector<CategoryData>());
  boost::shared_ptr<std::vector<ImageData> > imageDataVector(new std::vector<ImageData>());
  boost::shared_ptr<StringToVectorOfStrings> stringToVectorOfStrings(new StringToVectorOfStrings());

  // Load the COCO Annotations
  if(missingFiles)
  {
    std::cout << "Processing coco json annotation files.. this may take a while..\n";
    process_annotation_file(COCO_2014, VOC_TRAIN);
    process_annotation_file(COCO_2014, VOC_VAL);

    std::cout << "Converting hash maps to vectors for serialization..\n";
    *annotationDataVector = Util::to_vector(*m_annotationDataHash);
    *categoryDataVector = Util::to_vector(*m_categoryDataHash);
    *imageDataVector = Util::to_vector(*m_imageDataHash);

    // Serialize the data vectors to files.
    std::cout << "Saving..\n";
    SerializationUtil::save_binary(expectedPaths[0], *annotationDataVector);
    SerializationUtil::save_binary(expectedPaths[1], *categoryDataVector);
    SerializationUtil::save_binary(expectedPaths[2], *imageDataVector);
    SerializationUtil::save_binary(expectedPaths[3], m_splitYearToImagePaths);
  }
  else
  {
    // Load the has maps from file.
    std::cout << "Loading coco data from files..\n";
    annotationDataVector = SerializationUtil::load_binary(expectedPaths[0], annotationDataVector);
    categoryDataVector = SerializationUtil::load_binary(expectedPaths[1], categoryDataVector);
    imageDataVector = SerializationUtil::load_binary(expectedPaths[2], imageDataVector);
    stringToVectorOfStrings = SerializationUtil::load_binary(expectedPaths[3], stringToVectorOfStrings);
    m_splitYearToImagePaths = *stringToVectorOfStrings;

    std::cout << "Creating hash maps..\n";
    *m_annotationDataHash = Util::to_hash(*annotationDataVector);
    *m_categoryDataHash = Util::to_hash(*categoryDataVector);
    *m_imageDataHash = Util::to_hash(*imageDataVector);
  }

  processAnnotationTime.stop();
  std::cout << "###" << processAnnotationTime << '\n' << std::endl;

  // Set the annotation category count.
  size_t counter(0);
  const size_t maxCOCOCategoryId(100);
  for(size_t i = 0; i < maxCOCOCategoryId; ++i)
  {
    CategoryDataHash::const_iterator it = m_categoryDataHash->find(i);
    if(it == m_categoryDataHash->end()) continue;
    std::string categoryName = it->second.categoryName;
    m_categories.push_back(categoryName);
    size_t categoryId = counter;
    m_categoryNameToId.insert(std::make_pair(categoryName, categoryId));
    m_categoryIdToName.insert(std::make_pair(categoryId, categoryName));
    m_cocoCategoryIds.push_back(it->first);
    m_cocoCategoryIdsToCategoryIds.insert(std::make_pair(it->first, categoryId));
    ++counter;
  }

  size_t categoryCount(m_categories.size());
  VOCAnnotation::set_category_count(categoryCount);
  VOCAnnotation::set_category_name_to_id(m_categoryNameToId);
  VOCAnnotation::set_category_id_to_name(m_categoryIdToName);
  COCOAnnotation::set_annotation_data_hash(m_annotationDataHash);
  COCOAnnotation::set_image_data_hash(m_imageDataHash);
  COCOAnnotation::set_coco_category_ids_to_category_ids(m_cocoCategoryIdsToCategoryIds);

  // Get all the image paths.
  std::vector<std::string> imagePaths = get_image_paths(COCO_ALLYEARS, VOC_TRAINVAL, VOC_JPEG);

  // Initialise the map from image names to object ids.
  for(size_t i = 0; i < imagePaths.size(); ++i)
  {
    std::string imageName = (boost::filesystem::path(imagePaths[i])).stem().string();
    m_imageNameToObjectIds.insert(std::make_pair(imageName, std::vector<size_t>()));
  }

  // Create a map from image names to object ids;
  AnnotationDataHash::const_iterator it = m_annotationDataHash->begin(), iend = m_annotationDataHash->end();
  for(; it != iend; ++it)
  {
    size_t objectId = it->first;
    size_t imageId = it->second.imageId;
    ImageDataHash::const_iterator imageIt = m_imageDataHash->find(imageId);
    if(imageIt == m_imageDataHash->end()) throw std::runtime_error("Cannot find image Id: " + boost::lexical_cast<std::string>(imageId));
    std::string imageName = imageIt->second.imageName;

    //Lookup the image Name in the map;
    std::map<std::string,std::vector<size_t> >::iterator mapIt = m_imageNameToObjectIds.find(imageName);
    if(mapIt == m_imageNameToObjectIds.end()) throw std::runtime_error("Cannot find image Name in the map: " + imageName);
    mapIt->second.push_back(objectId);
  }

  // Create a map from image names to annotation pointers.
  for(size_t i = 0, size = imagePaths.size(); i < size; ++i)
  {
    boost::filesystem::path imagePath(imagePaths[i]);
    std::string imageName = imagePath.stem().string();
    VOCAnnotation_Ptr annotation(new COCOAnnotation(imagePath.string(), MapUtil::lookup(m_imageNameToObjectIds, imageName)));
    m_imageNameToAnnotation.insert(std::make_pair(imageName,annotation));
  }
}

void COCODatasetInstance::process_annotation_file(VOCYear year, VOCSplit split)
{
  std::string yearName = get_year_name(year);
  std::string splitName = get_split_name(split);
  std::string splitYear = splitName + yearName;

  boost::property_tree::ptree tree = PropertyUtil::load_properties_from_json(get_annotation_file(year, split));

  // Process the annotation data.
  read_annotation_data(tree.get_child("annotations"));

  // Process the image data.
  read_image_data(tree.get_child("images"), splitYear);

  // Process the category data.
  read_category_data(tree.get_child("categories"));
}

COCODatasetInstance::FloatVector COCODatasetInstance::extract_float_vector(const boost::property_tree::ptree& tree)
{
  //std::cout << "children" << tree.size() << '\n';
  FloatVector array(tree.size());
  size_t counter(0);
  for(boost::property_tree::ptree::const_iterator it = tree.begin(), iend = tree.end(); it != iend; ++it)
  {
    ptree entry = it->second;
    array[counter++] = entry.get_value<float>();
  }

  return array;
}

COCODatasetInstance::FloatVectors COCODatasetInstance::annotation_to_polygons(const boost::property_tree::ptree& tree)
{
  //std::cout << "number of polygons" << tree.size() << '\n';
  FloatVectors polygons(tree.size());
  size_t counter(0);
  for(boost::property_tree::ptree::const_iterator it = tree.begin(), iend = tree.end(); it != iend; ++it)
  {
    ptree entry = it->second;
    polygons[counter] = extract_float_vector(entry.get_child(""));
  }

  return polygons;
}

void COCODatasetInstance::read_annotation_data(const boost::property_tree::ptree& tree)
{
  for(boost::property_tree::ptree::const_iterator it = tree.begin(), iend = tree.end(); it != iend; ++it)
  {
    AnnotationData annData;

    ptree entry = it->second;

    size_t iscrowd = entry.get<size_t>("iscrowd");
    if(iscrowd) continue;

    annData.id = entry.get<size_t>("id");
    annData.imageId = entry.get<size_t>("image_id");
    annData.categoryId = entry.get<size_t>("category_id");
    annData.bbox = extract_float_vector(entry.get_child("bbox"));
    annData.polygons = annotation_to_polygons(entry.get_child("segmentation"));

    m_annotationDataHash->insert(std::make_pair(annData.id, annData));
  }
}

void COCODatasetInstance::read_category_data(const boost::property_tree::ptree& tree)
{
  for(boost::property_tree::ptree::const_iterator it = tree.begin(), iend = tree.end(); it != iend; ++it)
  {
    CategoryData catData;

    ptree entry = it->second;

    catData.categoryName = entry.get<std::string>("name");
    catData.categorySuperClass = entry.get<std::string>("supercategory");
    catData.id = entry.get<size_t>("id");

    m_categoryDataHash->insert(std::make_pair(catData.id, catData));
  }
}

void COCODatasetInstance::read_image_data(const boost::property_tree::ptree& tree, const std::string& splitYear)
{
  std::vector<std::string> imagePaths;
  for(boost::property_tree::ptree::const_iterator it = tree.begin(), iend = tree.end(); it != iend; ++it)
  {
    ImageData imData;
    ptree entry = it->second;

    std::string fileName = entry.get<std::string>("file_name");
    imData.imageName = (boost::filesystem::path(fileName)).stem().string();
    imData.id = entry.get<size_t>("id");
    imData.height = entry.get<size_t>("height");
    imData.width = entry.get<size_t>("width");

    m_imageDataHash->insert(std::make_pair(imData.id, imData));

    // FIXME MOve this bit outside the data loading
    std::string imagePath = m_rootDir + '/' + splitYear + '/' + fileName;
    imagePaths.push_back(imagePath);
  }

  // FIXME MOve this bit outside the data loading
  m_splitYearToImagePaths.insert(std::make_pair(splitYear, imagePaths));
}

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const COCODatasetInstance& d)
{
  os << "COCO Dataset\n";
  return os;
}
