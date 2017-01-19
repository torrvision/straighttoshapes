/**
 * vanilla: VOCAnnotation.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "VOCAnnotation.h"

#include <iostream>
#include <algorithm>

#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>

#include <opencv2/highgui/highgui.hpp>

//#################### PRIVATE STATIC MEMBER VARIABLES ####################

size_t VOCAnnotation::m_categoryCount = 0;
std::map<std::string,size_t> VOCAnnotation::m_categoryNameToId = std::map<std::string,size_t>();
std::map<size_t,std::string> VOCAnnotation::m_categoryIdToName = std::map<size_t,std::string>();
boost::unordered_map<cv::Vec3b,size_t,Vec3bHash> VOCAnnotation::m_colourToCategoryIdHash = boost::unordered_map<cv::Vec3b,size_t,Vec3bHash>();

//#################### CONSTRUCTORS ####################

VOCAnnotation::VOCAnnotation()
{}

VOCAnnotation::VOCAnnotation(const std::string& imageName_, const Size& size_)
: imageName(imageName_),
  size(size_)
{}

//#################### DESTRUCTORS ####################

VOCAnnotation::~VOCAnnotation()
{}

//#################### PUBLIC MEMBER FUNCTIONS ####################

void VOCAnnotation::save(const std::string& saveDir) const
{
  boost::format threeDigits("%03d");
  const std::vector<VOCObject>& objects = get_objects();
  for(size_t i = 0; i < objects.size(); ++i)
  {
    const VOCObject& obj = objects[i];
    std::string categoryName(obj.categoryName);
    // Make sure thte categoryName has no spaces.
    categoryName.erase(std::remove_if(categoryName.begin(), categoryName.end(), ::isspace), categoryName.end());
    size_t instanceId = i;
    std::string fileName = this->imageName + '_' + categoryName  + '_' + "inst" + (threeDigits % instanceId).str() + ".png";
    std::string filePath = saveDir + '/' + fileName;
    std::cout << "Saving: " << filePath << '\n';
    // Save as png with least compression
    if(obj.rep.get_mask().data)
    {
      cv::imwrite(filePath, obj.rep.get_mask(), boost::assign::list_of(0));
    }
  }
}

//#################### PUBLIC STATIC MEMBER FUNCTIONS ####################

size_t VOCAnnotation::get_category_count()
{
  return m_categoryCount;
}

const std::map<std::string,size_t>& VOCAnnotation::get_category_name_to_id()
{
  return m_categoryNameToId;
}

const std::map<size_t,std::string>& VOCAnnotation::get_category_id_to_name()
{
  return m_categoryIdToName;
}

const boost::unordered_map<cv::Vec3b,size_t,Vec3bHash>& VOCAnnotation::get_colour_to_category_id_hash()
{
  return m_colourToCategoryIdHash;
}

void VOCAnnotation::set_category_count(size_t categoryCount)
{
  m_categoryCount = categoryCount;
}

void VOCAnnotation::set_category_id_to_name(const std::map<size_t,std::string>& categoryIdToName)
{
  m_categoryIdToName = categoryIdToName;
}

void VOCAnnotation::set_category_name_to_id(const std::map<std::string,size_t>& categoryNameToId)
{
  m_categoryNameToId = categoryNameToId;
}

void VOCAnnotation::set_colour_to_category_id_hash(const boost::unordered_map<cv::Vec3b,size_t,Vec3bHash>& colourToCategoryIdHash)
{
  m_colourToCategoryIdHash = colourToCategoryIdHash;
}

size_t VOCAnnotation::calculate_object_count(const std::vector<VOCObject>& objects, size_t categoryId)
{
  size_t objectCount(0);
  for(size_t i = 0, size = objects.size(); i < size; ++i)
  {
    if(categoryId == static_cast<size_t>(objects[i].categoryId))
    {
      if(!objects[i].difficult) ++objectCount;
    }
  }

  return objectCount;
}

//#################### OUTPUT #################### 

std::ostream& operator<<(std::ostream& os, const VOCAnnotation& a)
{
  os << "ID: " << a.imageName << '\n';
  os << "Size: " << a.size << '\n';
  return os;
}

