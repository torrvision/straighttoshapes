/**
 * vanilla: VOCSegmentationAnnotation.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "VOCSegmentationAnnotation.h"

#include "../Util.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>

#include <tvgutil/containers/MapUtil.h>
using namespace tvgutil;

//#################### CONSTRUCTORS ####################

VOCSegmentationAnnotation::VOCSegmentationAnnotation(const std::string& segmentationClassAnnotationPath, const std::string& segmentationObjectAnnotationPath)
: VOCAnnotation(),
  m_segmentationClassAnnotationPath(segmentationClassAnnotationPath),
  m_segmentationObjectAnnotationPath(segmentationObjectAnnotationPath)
{
  read_annotation(segmentationClassAnnotationPath);
}

//#################### PUBLIC MEMBER FUNCTIONS ####################

std::vector<VOCObject> VOCSegmentationAnnotation::get_objects(const boost::optional<DataTransformation>& dataTransformation) const
{
  cv::Mat3b objectSegmentationColourMap = load_object_annotation();
  cv::Mat1b objectSegmentation = Util::convert_colourmap_to_category(objectSegmentationColourMap, VOCAnnotation::get_colour_to_category_id_hash());
  if(dataTransformation) objectSegmentation = (*dataTransformation).apply_label_image_transformation(objectSegmentation);
  std::set<uint8_t> idsToIgnore;
  idsToIgnore.insert(0);  // background
  idsToIgnore.insert(255);// void
  std::vector<cv::Mat1b> objectMasks = Util::unique_segments_to_binary_masks(objectSegmentation, idsToIgnore);


  cv::Mat3b categorySegmentationColourMap = load_class_annotation();
  cv::Mat1b categorySegmentation = Util::convert_colourmap_to_category(categorySegmentationColourMap, VOCAnnotation::get_colour_to_category_id_hash());
  if(dataTransformation) categorySegmentation = (*dataTransformation).apply_label_image_transformation(categorySegmentation);
  std::vector<cv::Mat1b> segmentMasks = Util::segment_ids_to_binary_mask_channels(categorySegmentation);

  // Extra step to cope with the strange format of the SBD dataset.
  cv::Mat1b backgroundObjectSegmentation = (objectSegmentation == 0);
  cv::Mat1b foregroundCategorySegmentation = ((categorySegmentation != 0) & (categorySegmentation != 255));
  cv::Mat1b firstObjectSBD = backgroundObjectSegmentation & foregroundCategorySegmentation;
  if(cv::countNonZero(firstObjectSBD))
  {
    objectMasks.push_back(firstObjectSBD);
  }

#if 0
  cv::imshow("backgroundObjectSegmentation", backgroundObjectSegmentation);
  cv::imshow("foregroundCategorySegmentation", foregroundCategorySegmentation);
  cv::imshow("firstObjectSBD", firstObjectSBD);
  for(size_t i = 0; i < objectMasks.size(); ++i)
  {
    cv::imshow("objectMask" + boost::lexical_cast<std::string>(i), objectMasks[i]);
  }

  for(size_t j = 0; j < segmentMasks.size(); ++j)
  {
    if(segmentMasks[j].data)
      cv::imshow("segmentMask" + boost::lexical_cast<std::string>(j), segmentMasks[j]);
  }

  cv::waitKey();
#endif

  std::vector<VOCObject> objects;
  // Ignore the background and void labels.
  for(size_t i = 0; i < objectMasks.size(); ++i)
  {
    // Ignore the background, and the void label has not been included in the set.
    for(size_t j = 1; j < segmentMasks.size(); ++j)
    {
      if(segmentMasks[j].data)
      {
        cv::Mat1b intersectionMask = cv::Mat::zeros(objectMasks[0].rows, objectMasks[0].cols, CV_8UC1);
        cv::bitwise_and(objectMasks[i], segmentMasks[j], intersectionMask, objectMasks[i]);
        int pixelCount = cv::countNonZero(intersectionMask);

#if 0
        cv::imshow("intersectionMask", intersectionMask);
        std::cout << "pixelCount: " << pixelCount << std::endl;
        cv::imshow("objectMask" + boost::lexical_cast<std::string>(i), objectMasks[i]);
        cv::imshow("segmentMask" + boost::lexical_cast<std::string>(j), segmentMasks[j]);
        cv::waitKey();
#endif
        const int minBoxWidth(4);
        const int minBoxHeight(4);
        const int minPixelsInMask(10);

        if(pixelCount > minPixelsInMask)
        {
          //std::cout << "assigned object: " << i << " to segment: " << j << std::endl;
          cv::Mat1b mask = objectMasks[i].clone();
          VOCBox vbox = Util::mask_to_vocbox(mask);
          if((vbox.w() > minBoxWidth) && (vbox.h() > minBoxHeight))
          {
            cv::Mat1b croppedMask = mask(Util::to_rect(vbox)).clone();
#if 0
            cv::imshow("croppedMask", croppedMask);
            cv::waitKey();
#endif

            if(dataTransformation)
            {
              vbox = (*dataTransformation).apply_scale_and_clip_to_network_size(vbox, Size(objectSegmentationColourMap.cols, objectSegmentationColourMap.rows, 1));
            }

            Shape shape(vbox, croppedMask);
            const bool isDifficult(false);
            size_t categoryId = j-1;
            std::string categoryName = MapUtil::lookup(VOCAnnotation::get_category_id_to_name(), categoryId);
            VOCObject vObject(isDifficult,
                              shape,
                              categoryName,
                              categoryId);
            objects.push_back(vObject);
            break;
          }
        }
      }

    }
  }
  return objects;
}

cv::Mat3b VOCSegmentationAnnotation::load_class_annotation() const
{
  return cv::imread(m_segmentationClassAnnotationPath, CV_LOAD_IMAGE_COLOR);
}

cv::Mat3b VOCSegmentationAnnotation::load_object_annotation() const
{
  return cv::imread(m_segmentationObjectAnnotationPath, CV_LOAD_IMAGE_COLOR);
}

/*
void VOCSegmentationAnnotation::save(const std::string& saveDir) const
{
  boost::format threeDigits("%03d");
  const std::vector<VOCObject>& objects = get_objects();
  for(size_t i = 0; i < objects.size(); ++i)
  {
    const VOCObject& obj = objects[i];
    size_t instanceId = i;
    std::string fileName = this->imageName + '_' + obj.categoryName  + '_' + "inst" + (threeDigits % instanceId).str() + ".png";
    std::string filePath = saveDir + '/' + fileName;
    std::cout << "Saving: " << filePath << '\n';
    // Save as png with least compression
    cv::imwrite(filePath, obj.rep.get_mask(), boost::assign::list_of(0));
  }
}
*/

//#################### PRIVATE MEMBER FUNCTIONS ####################

void  VOCSegmentationAnnotation::read_annotation(const std::string& path)
{
  VOCAnnotation::imageName = (boost::filesystem::path(path)).stem().string();
  //std::cout << "TODO: read segmentation annotation\n";
}

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const VOCSegmentationAnnotation& a)
{
  os << "Segmentation representation: TODO" << '\n';
  return os;
}

