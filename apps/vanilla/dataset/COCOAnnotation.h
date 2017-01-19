/**
 * vanilla: COCOAnnotation.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2015. All rights reserved.
 */

#ifndef H_VANILLA_COCOANNOTATION
#define H_VANILLA_COCOANNOTATION

#include "VOCAnnotation.h"

#include <opencv2/core/core.hpp>

#include <boost/shared_ptr.hpp>

#include "COCODataPrimitives.h"

class COCOAnnotation : public VOCAnnotation
{
  //#################### PRIVATE TYPEDEFS ####################
private:
  typedef boost::unordered_map<size_t, AnnotationData> AnnotationDataHash;
  typedef boost::shared_ptr<AnnotationDataHash> AnnotationDataHash_Ptr;
  typedef boost::shared_ptr<const AnnotationDataHash> AnnotationDataHash_CPtr;

  typedef boost::unordered_map<size_t, ImageData> ImageDataHash;
  typedef boost::shared_ptr<ImageDataHash> ImageDataHash_Ptr;
  typedef boost::shared_ptr<const ImageDataHash> ImageDataHash_CPtr;

  //#################### PRIVATE MEMBER VARIABLES ####################
private:
  std::vector<size_t> m_objectIds;
  static AnnotationDataHash_CPtr m_annotationDataHash;
  static ImageDataHash_CPtr m_imageDataHash;
  static std::map<size_t,size_t> m_cocoCategoryIdsToCategoryIds;

  //#################### CONSTRUCTORS ####################
public:
  COCOAnnotation(const std::string& imagePath, const std::vector<size_t>& objectIds);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  /** Override. */
  virtual std::vector<VOCObject> get_objects(const boost::optional<DataTransformation>& dataTransformation = boost::none) const;

  static void set_annotation_data_hash(const AnnotationDataHash_CPtr& annotationDataHash);
  static void set_image_data_hash(const ImageDataHash_CPtr& imageDataHash);
  static void set_coco_category_ids_to_category_ids(const std::map<size_t,size_t>& cocoCategoryIdsToCategoryIds);

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  /** Override. */
  void read_annotation(const std::string& path);
};

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const COCOAnnotation& annotation);

//#################### TYPEDEFS ####################

typedef boost::shared_ptr<COCOAnnotation> COCOAnnotation_Ptr;
typedef boost::shared_ptr<const COCOAnnotation> COCOAnnotation_CPtr;

#endif
