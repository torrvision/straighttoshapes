/**
 * vanilla: VOCDetectionAnnotation.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "VOCDetectionAnnotation.h"

#include "../core/VOCBox.h"

#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <boost/property_tree/xml_parser.hpp>

#include <tvgutil/containers/LimitedContainer.h>
#include <tvgutil/containers/MapUtil.h>
#include <tvgutil/persistence/PropertyUtil.h>
using namespace tvgutil;

//#################### CONSTRUCTORS ####################

VOCDetectionAnnotation::VOCDetectionAnnotation(const std::string& annotationPath)
: VOCAnnotation()
{
  read_annotation(annotationPath);
}

//#################### PUBLIC MEMBER FUNCTIONS #################### 

std::vector<VOCObject> VOCDetectionAnnotation::get_objects(const boost::optional<DataTransformation>& dataTransformation) const
{
  if(!dataTransformation) return m_objects;

  std::vector<VOCObject> transformedObjects;
  for(size_t i = 0; i < m_objects.size(); ++i)
  {
    VOCBox vbox = (*dataTransformation).apply_transformation(m_objects[i].rep.get_voc_box(), size);

    VOCObject vObject(m_objects[i].difficult,
                      Shape(vbox),
                      m_objects[i].categoryName,
                      m_objects[i].categoryId);

    transformedObjects.push_back(vObject);
  }

  return transformedObjects;
}

/*
void VOCDetectionAnnotation::save(const std::string& saveDir) const
{
  // TODO;
  throw std::runtime_error("Not implemented yet");
}
*/

//#################### PRIVATE MEMBER FUNCTIONS #################### 

void  VOCDetectionAnnotation::read_annotation(const std::string& path)
{
  boost::property_tree::ptree xmlAnnotation = PropertyUtil::load_properties_from_xml(path);
  VOCAnnotation::imageName = (boost::filesystem::path(path)).stem().string();
  std::string filename = xmlAnnotation.get<std::string>("annotation.filename");
  if(imageName != ((boost::filesystem::path(filename)).stem().string()))
    throw std::runtime_error("Mismatch..");

  int width = xmlAnnotation.get<int>("annotation.size.width");
  int height = xmlAnnotation.get<int>("annotation.size.height");
  int depth = xmlAnnotation.get<int>("annotation.size.depth");
  VOCAnnotation::size = Size(width, height, depth);

  BOOST_FOREACH(boost::property_tree::ptree::value_type const &v, xmlAnnotation.get_child("annotation") )
  {
    if(v.first == "object")
    { 
      std::string name = v.second.get<std::string>("name");
      bool isDifficult = v.second.get<bool>("difficult");
      int xmin = v.second.get<int>("bndbox.xmin");
      int ymin = v.second.get<int>("bndbox.ymin");
      int xmax = v.second.get<int>("bndbox.xmax");
      int ymax = v.second.get<int>("bndbox.ymax");

      size_t id = MapUtil::lookup(VOCAnnotation::get_category_name_to_id(), name);
      if(id >= m_categoryCount) throw std::runtime_error("id exceeds the number of categories");

      Shape shape(VOCBox(xmin, ymin, xmax, ymax));
      m_objects.push_back(VOCObject(isDifficult, shape, name, id));
    }
  }
}

//#################### OUTPUT #################### 

std::ostream& operator<<(std::ostream& os, const VOCDetectionAnnotation& a)
{
  os << "Objects: " << make_limited_container(a.get_objects(), 20) << '\n';
  return os;
}

