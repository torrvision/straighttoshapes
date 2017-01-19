/**
 * vanilla: COCOAnnotation.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "COCOAnnotation.h"

#include "../Util.h"

#include <boost/format.hpp>
#include <boost/assign/list_of.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>

#include <tvgutil/containers/MapUtil.h>
#include <tvgutil/containers/LimitedContainer.h>
using namespace tvgutil;

//#################### PRIVATE STATIC MEMBER VARIABLES ####################

COCOAnnotation::AnnotationDataHash_CPtr COCOAnnotation::m_annotationDataHash = AnnotationDataHash_CPtr(new AnnotationDataHash);
COCOAnnotation::ImageDataHash_CPtr COCOAnnotation::m_imageDataHash = ImageDataHash_CPtr(new ImageDataHash);
std::map<size_t,size_t> COCOAnnotation::m_cocoCategoryIdsToCategoryIds = std::map<size_t,size_t>();

//#################### CONSTRUCTORS ####################

COCOAnnotation::COCOAnnotation(const std::string& imagePath, const std::vector<size_t>& objectIds)
: VOCAnnotation(),
  m_objectIds(objectIds)
{
  read_annotation(imagePath);
}

//#################### PUBLIC MEMBER FUNCTIONS ####################

std::vector<VOCObject> COCOAnnotation::get_objects(const boost::optional<DataTransformation>& dataTransformation) const
{
  std::vector<VOCObject> objects;
  for(size_t i = 0; i < m_objectIds.size(); ++i)
  {
    // Get the objects in this image from the hash map.
    AnnotationDataHash::const_iterator it = m_annotationDataHash->find(m_objectIds[i]);
    if(it == m_annotationDataHash->end()) throw std::runtime_error("Could not find object in hash table");
    const AnnotationData& annData = it->second;

    //size_t cocoCategoryId = boost::get<2>(annData);
    // Get the category Id.
    size_t categoryId = MapUtil::lookup(m_cocoCategoryIdsToCategoryIds, annData.categoryId);

    // Get the category name.
    std::string categoryName = MapUtil::lookup(m_categoryIdToName, categoryId);

    // Create a mask canvas of the same size as the input image.
    ImageDataHash::const_iterator imageIt = m_imageDataHash->find(annData.imageId);
    if(imageIt == m_imageDataHash->end()) throw std::runtime_error("Could not find image in hash table");
    const ImageData& imageData = imageIt->second;
    cv::Mat1b mask = cv::Mat::zeros(imageData.height, imageData.width, CV_8UC1);

    // Draw the binary mask and transform if necessary.
    std::vector<std::vector<float> > polygons = annData.polygons;// boost::get<3>(annData);
    std::vector<std::vector<cv::Point> > contours = Util::to_contours(polygons);
    cv::drawContours(mask, contours, -1, cv::Scalar(255), cv::FILLED);
    if(dataTransformation) mask = (*dataTransformation).apply_label_image_transformation(mask);
#if 0
    cv::imshow("mask",mask);
    cv::waitKey();
#endif

    // Get the box data.
#if 0 // FIXME check if there is a difference between the boxes..
    const std::vector<float>& bbox = annData.bbox;// boost::get<4>(annData);

    int xmin(static_cast<int>(bbox[0])); // x top left corner
    int ymin(static_cast<int>(bbox[1])); // y top left corner
    int xmax(static_cast<int>(bbox[0] + bbox[2])); // bbox[3] width
    int ymax(static_cast<int>(bbox[1] + bbox[3])); // bbox[4] height
    VOCBox vbox(xmin, ymin, xmax, ymax);

    if(dataTransformation)
    {
      vbox = (*dataTransformation).apply_scale_and_clip_to_network_size(vbox, Size(mask.cols, mask.rows, 1));
    }
#endif

    const int minBoxWidth(4);
    const int minBoxHeight(4);
    const int minPixelsInMask(10);
    int pixelCount = cv::countNonZero(mask);
    if(pixelCount > minPixelsInMask)
    {
      VOCBox vbox = Util::mask_to_vocbox(mask);
      if((vbox.w() > minBoxWidth) && (vbox.h() > minBoxHeight))
      {
        cv::Mat1b croppedMask = mask(Util::to_rect(vbox)).clone();
        if(dataTransformation)
        {
          vbox = (*dataTransformation).apply_scale_and_clip_to_network_size(vbox, Size(mask.cols, mask.rows, 1));
        }


        const bool isDifficult(false);
        VOCObject object(isDifficult, Shape(vbox, croppedMask), categoryName, categoryId);
        objects.push_back(object);
      }
    }
  }

  return objects;
}

//#################### PRIVATE MEMBER FUNCTIONS ####################

void COCOAnnotation::set_annotation_data_hash(const AnnotationDataHash_CPtr& annotationDataHash)
{
  m_annotationDataHash = annotationDataHash;
}

void COCOAnnotation::set_image_data_hash(const ImageDataHash_CPtr& imageDataHash)
{
  m_imageDataHash = imageDataHash;
}

void COCOAnnotation::set_coco_category_ids_to_category_ids(const std::map<size_t,size_t>& cocoCategoryIdsToCategoryIds)
{
  m_cocoCategoryIdsToCategoryIds = cocoCategoryIdsToCategoryIds;
}

void  COCOAnnotation::read_annotation(const std::string& path)
{
  VOCAnnotation::imageName = (boost::filesystem::path(path)).stem().string();
  //std::cout << "TODO: read segmentation annotation\n";
}

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const COCOAnnotation& a)
{
  os << "COCO Segmentation representation: TODO" << '\n';
  return os;
}

