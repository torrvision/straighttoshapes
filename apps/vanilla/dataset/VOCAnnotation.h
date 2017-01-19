/**
 * vanilla: VOCAnnotation.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_VOCANNOTATION
#define H_VANILLA_VOCANNOTATION

#include "../core/Size.h"
#include "../core/VOCObject.h"
#include "../data/DataTransformation.h"
#include "../Util.h"

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include <map>
#include <string>
#include <vector>

class VOCAnnotation
{
  //#################### PROTECTED STATIC MEMBER VARIABLES #################### 
protected:
  static size_t m_categoryCount;
  static std::map<std::string,size_t> m_categoryNameToId;
  static std::map<size_t,std::string> m_categoryIdToName;
  static boost::unordered_map<cv::Vec3b,size_t,Vec3bHash> m_colourToCategoryIdHash;

  //#################### PUBLIC MEMBER VARIABLES #################### 
public:
  std::string imageName;
  Size size;

  //#################### CONSTRUCTORS #################### 
public:
  VOCAnnotation();
  VOCAnnotation(const std::string& imageName_, const Size& size_);

  //#################### DESTRUCTORS #################### 
public:
  virtual ~VOCAnnotation();

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  virtual std::vector<VOCObject> get_objects(const boost::optional<DataTransformation>& dataTransformation = boost::none) const = 0;

  void save(const std::string& saveDir) const;

  //#################### PUBLIC STATIC MEMBER FUNCTIONS #################### 
public:
  static size_t get_category_count();
  static const std::map<std::string,size_t>& get_category_name_to_id();
  static const std::map<size_t, std::string>& get_category_id_to_name();
  static const boost::unordered_map<cv::Vec3b,size_t,Vec3bHash>& get_colour_to_category_id_hash();
  static void set_category_count(size_t categoryCount);
  static void set_category_name_to_id(const std::map<std::string,size_t>& categoryNameToId);
  static void set_category_id_to_name(const std::map<size_t,std::string>& categoryIdToName);
  static void set_colour_to_category_id_hash(const boost::unordered_map<cv::Vec3b,size_t,Vec3bHash>& colourToCategoryIdHash);
  static size_t calculate_object_count(const std::vector<VOCObject>& objects, size_t categoryId);

  //#################### PRIVATE MEMBER FUNCTIONS #################### 
private:
  virtual void read_annotation(const std::string& path) = 0;
};

//#################### OUTPUT #################### 

std::ostream& operator<<(std::ostream& os, const VOCAnnotation& annotation);

typedef boost::shared_ptr<VOCAnnotation> VOCAnnotation_Ptr;
typedef boost::shared_ptr<const VOCAnnotation> VOCAnnotation_CPtr;

#endif
