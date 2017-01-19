/**
 * vanilla: COCODataPrimitives.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2015. All rights reserved.
 */

#ifndef H_VANILLA_COCODATAPRIMITIVES
#define H_VANILLA_COCODATAPRIMITIVES

#include <boost/serialization/serialization.hpp>

typedef std::vector<float> FloatVector;
typedef std::vector<FloatVector> FloatVectors;

struct AnnotationData
{
  size_t id;
  size_t imageId;
  size_t categoryId;
  FloatVectors polygons;
  FloatVector bbox;

  //#################### SERIALIZATION ####################
  /**
   * \brief Serializes the annotation data to/from an archive.
   */
  template <typename Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & id;
    ar & imageId;
    ar & categoryId;
    ar & polygons;
    ar & bbox;
  }
};

struct CategoryData
{
  size_t id;
  std::string categoryName;
  std::string categorySuperClass;

  //#################### SERIALIZATION ####################
  /**
   * \brief Serializes the category data to/from an archive.
   */
  template <typename Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & id;
    ar & categoryName;
    ar & categorySuperClass;
  }
};

struct ImageData
{
  size_t id;
  size_t width;
  size_t height;
  std::string imageName;

  //#################### SERIALIZATION ####################
  /**
   * \brief Serializes the image data to/from an archive.
   */
  template <typename Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    ar & id;
    ar & width;
    ar & height;
    ar & imageName;
  }
};


#endif
