/**
 * vanilla: TrainingDataGenerator.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_BOXTRAININGDATAGENERATOR
#define H_VANILLA_BOXTRAININGDATAGENERATOR

#include "DataTransformationFactory.h"

#include "../core/Datum.h"
#include "../core/Detection.h"
#include "../core/DetectionSettings.h"
#include "../dataset/Dataset.h"

#include <tvgutil/containers/CircularQueue.h>
#include <tvgutil/numbers/RandomNumberGenerator.h>
#include <tvgutil/statistics/Histogram.h>

#include <tvgshape/ShapeDescriptorCalculator.h>

/**
 * \brief TODO.
 */
class TrainingDataGenerator
{
  //#################### PRIVATE TYPEDEFS ####################
private:
  typedef boost::shared_ptr<tvgutil::CircularQueue<std::vector<Datum> > > CQ_Ptr;

  //#################### PRIVATE VARIABLES ####################
private:
  Dataset_CPtr m_dataset;

  DataTransformationFactory m_dataTransformationFactory;

  size_t m_imageHeightNetwork;

  std::vector<std::string> m_imagePaths;

  size_t m_imageWidthNetwork;

  mutable tvgutil::RandomNumberGenerator m_rng;

  mutable tvgutil::Histogram<std::string> m_hist;

  boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr> m_shapeDescriptorCalculator;

  //#################### CONSTRUCTORS ####################
public:
  TrainingDataGenerator(const std::vector<std::string>& imagePaths, const Dataset_CPtr& dataset, size_t imageWidthNetwork, size_t imageHeightNetwork, const DataTransformationFactory::Settings& dataTransformationSettings, unsigned int seed, const boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator = boost::none);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  void debug_training_datum(const Datum& datum, size_t imagesPerDatum, const DetectionSettings& ds) const;

  std::vector<Datum> generate_training_data(size_t numDatum, size_t imagesPerDatum, float jitter, const DetectionSettings& ds) const;

  Datum generate_training_datum(size_t imagesPerDatum, float jitter, const DetectionSettings& ds) const;

  void run_load_loop(CQ_Ptr& cq, size_t startBatchNumber, size_t maxBatchNumber, size_t numDatum, size_t imagesPerDatum, float jitter, const DetectionSettings& ds) const;

  friend std::ostream& operator<<(std::ostream& os, const TrainingDataGenerator& d);

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  std::vector<std::string> generate_random_image_paths(size_t imagesPerDatum) const;

  std::vector<float> prepare_input(const std::vector<std::string>& imagePaths, const std::vector<DataTransformation>& dataTransformations) const;

  std::vector<float> prepare_target(const std::vector<std::string>& imagePaths, const DetectionSettings& ds, const std::vector<DataTransformation>& dataTransformations) const;

  std::vector<float> create_target_from_detections(const Detections& detections, const DetectionSettings& ds) const;

  Detections create_detections_from_target(const std::vector<float>& target, const DetectionSettings& ds) const;

  std::vector<std::string> pick_paths(const std::vector<std::string>& randomPaths, size_t maximumSize) const;

  tvgutil::Histogram<std::string> histogram_add(const tvgutil::Histogram<std::string>& a, const tvgutil::Histogram<std::string>& b) const;
};


//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const TrainingDataGenerator& d);


#endif
