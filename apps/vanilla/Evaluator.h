/**
 * vanilla: Evaluator.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016, All rights reserved.
 */

#ifndef H_VANILLA_EVALUATOR
#define H_VANILLA_EVALUATOR

#include "core/Detection.h"
#include "core/DetectionSettings.h"
#include "dataset/Dataset.h"

#include <darknet/network.h>

#include <tvgshape/ShapeDescriptorCalculator.h>

/**
 * \brief TODO.
 */
class Evaluator
{
  //#################### TYPEDEFS ####################
private:
  typedef boost::unordered_map<std::string,std::vector<VOCObject> > NameToVOCObjectsHash;
  typedef std::vector<NamedCategoryDetection> NamedCategoryDetections;

  //#################### PRIVATE MEMBER VARIABLES ####################
private:
  Dataset_CPtr m_dataset;
  DetectionSettings m_ds;
  boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr> m_shapeDescriptorCalculator;
  
  //#################### CONSTRUCTORS ####################
public:
  Evaluator(const Dataset_CPtr& dataset, const DetectionSettings& ds, const boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator = boost::none);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  /** Calculate the mean average precision for a particular overlap threshold. */
  double calculate_map(network& net, const std::string& saveResultsPath, VOCYear vocYear, VOCSplit vocSplit, const std::string& uniqueStamp, double overlapThreshold = 0.5, const boost::optional<size_t>& maxImages = boost::none) const;

  /** Calculate the mean average precision over a set of overlap thresholds. */
  double calculate_map_vol(network& net, const std::string& saveResultsPath, VOCYear vocYear, VOCSplit vocSplit, const std::string& uniqueStamp, const std::vector<double>& overlapThresholds, const boost::optional<size_t>& maxImages = boost::none) const;

  /** Find the best and worst detections and save them to file. */
  void find_save_best_worst(network& net, const std::string& saveResultsPath, VOCYear year, VOCSplit split) const;

#if 0
  double calculate_map_matlab(network& net, const std::string& saveResultsPath, VOCYear vocYear, VOCSplit vocSplit, const std::string& uniqueStamp) const;
#endif

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  /** Calculate the average precisions for a set of categories. */
  std::vector<double> calculate_ap_for_categories(const NameToVOCObjectsHash& nameToVOCObjectsHash, const std::vector<NamedCategoryDetections>& namedCategoryDetections, double overlapThreshold) const;

  /** Calculate the average precision for a particular category. */
  double calculate_ap_for_category(const NameToVOCObjectsHash& nameToVOCObjectsHash, const NamedCategoryDetections& namedCategoryDetections, size_t categoryId, double overlapThreshold) const;

  /** Calculate the mean average precision from formatted detections and ground truth (for a particular overlap threshold.) */
  double calculate_map(const std::vector<NamedCategoryDetections>& perClassNamedCategoryDetections, const NameToVOCObjectsHash& nameToVOCObjectsHash, const std::string& saveResultsPath, const std::string uniqueStamp, double overlapThreshold = 0.5) const;

#if 0
  double calculate_map_matlab(const std::string& resultsPath, const std::string& competitionCode, const std::string& splitName) const;
#endif

  std::vector<std::vector<NamedCategoryDetection> > convert_to_named_category_detections(const std::vector<std::string>& imagePaths, const std::vector<Detections>& detections, double minDetectionScoreThreshold = 1e-5) const;

  NameToVOCObjectsHash create_name_to_objects_hash(const std::vector<std::string>& imagePaths) const;

  std::vector<std::string> get_save_results_per_category_files(const std::string& saveResultsPath, const std::string& competitionCode) const;

  double get_score_per_image(const NameToVOCObjectsHash& nameToVOCObjectsHash, const std::string& imagePath, const Detections& detections) const;

  void print_detections(const std::vector<std::vector<NamedCategoryDetection> >& namedCategoryDetections, const std::vector<std::string>& saveResultsPerCategoryFiles, size_t categoryCount) const;

  std::vector<NamedCategoryDetections> calculate_detections_per_category(network& net, const std::vector<std::string>& imagePaths) const;

  void save_images(const std::string& saveResultsPath, const std::string& imagePath, const Detections& detections, const std::string& tag) const;

  void save_results_to_file(const std::string& saveResultsPath, const std::string& uniqueStamp, const std::vector<std::string>& categoryNames, const std::vector<double>& ap, double overlapThreshold) const;

  void write_settings_report(const std::string& saveResultsPath, const std::string& uniqueStamp, VOCYear vocYear, VOCSplit vocSplit) const;

};

#endif
