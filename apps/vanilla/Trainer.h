/**
 * vanilla: Trainer.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016, All rights reserved.
 */

#ifndef H_VANILLA_TRAINER
#define H_VANILLA_TRAINER

#include "core/Datum.h"
#include "core/DetectionSettings.h"
#include "dataset/Dataset.h"

#include <darknet/network.h>

#include <boost/shared_ptr.hpp>

#include <evaluation/core/PerformanceTable.h>

#include <tvgutil/containers/CircularQueue.h>

#include <tvgshape/ShapeDescriptorCalculator.h>

/**
 * \brief TODO.
 */
class Trainer
{
  //#################### PRIVATE TYPEDEFS ####################
private:
  typedef boost::shared_ptr<tvgutil::CircularQueue<std::vector<Datum> > > CQ_Ptr;

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  Dataset_CPtr m_dataset;
  bool m_debugFlag;
  DetectionSettings m_ds;
  std::string m_experimentUniqueStamp;
  size_t m_seed;
  VOCYear m_year;
  boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr> m_shapeDescriptorCalculator;
  boost::optional<size_t> m_maxImagesToEvaluateOn;

  //#################### CONSTRUCTORS ####################
public:
  Trainer(const Dataset_CPtr& dataset, VOCYear year, const DetectionSettings& ds, const std::string& experimentUniqueStamp, bool debugFlag, size_t seed, const boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator = boost::none, const boost::optional<size_t>& maxImagesToEvaluateOn = boost::none);

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  void train(network& net, size_t epochCount) const;

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  void evaluate_network(network& net, size_t batchNumber, float epoch, evaluation::PerformanceTable& table, std::ofstream& reportEvaluation, const std::string& reportEvaluationFile) const;
  float get_and_save_performance(network& net, VOCYear vocYear, VOCSplit vocSplit, size_t batchNumber) const;
  float train_network(network& net, Datum& datum) const;
  float train_network(network& net, std::vector<Datum>& data) const;
};

#endif
