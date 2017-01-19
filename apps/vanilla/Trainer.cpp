/**
 * vanilla: Trainer.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016, All rights reserved.
 */

#include "Trainer.h"

#include "Evaluator.h"
#include "DarknetUtil.h"
#include "Util.h"

#include "data/TrainingDataGenerator.h"

#include "core/DetectionSettings.h"
#include "core/MovingAverage.h"

#include <fstream>

#include <boost/format.hpp>

#include <darknet/parser.h>

#include <evaluation/core/PerformanceMeasure.h>
using namespace evaluation;

#include <tvgutil/timing/Timer.h>
#include <tvgutil/timing/TimeUtil.h>
using namespace tvgutil;
using namespace tvgshape;

#include <tvgplot/PaletteGenerator.h>
#include <tvgplot/PlotWindow.h>
using namespace tvgplot;

//#################### CONSTRUCTORS ####################

Trainer::Trainer(const Dataset_CPtr& dataset, VOCYear year, const DetectionSettings& ds, const std::string& experimentUniqueStamp, bool debugFlag, size_t seed, const boost::optional<ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator, const boost::optional<size_t>& maxImagesToEvaluateOn)
: m_dataset(dataset),
  m_debugFlag(debugFlag),
  m_ds(ds),
  m_experimentUniqueStamp(experimentUniqueStamp),
  m_seed(seed),
  m_year(year),
  m_shapeDescriptorCalculator(shapeDescriptorCalculator),
  m_maxImagesToEvaluateOn(maxImagesToEvaluateOn)
{}

//#################### PUBLIC MEMBER FUNCTIONS ####################

//#define DEBUG_DATA
void Trainer::train(network& net, size_t epochCount) const
{
  boost::format sixDecimalPlaces("%0.6f");
  boost::format twoDecimalPlaces("%0.2f");
  boost::format sixDigits("%06d");
  boost::format tenDigits("%010d");

  std::string saveResultsDir = m_dataset->get_dir_in_results(m_experimentUniqueStamp);

  // Create report files for the experiment.
  std::string reportLossFile = saveResultsDir + "/reportLoss.txt";
  std::ofstream reportLoss(reportLossFile);

  std::string reportEvaluationFile = saveResultsDir + "/reportEvaluation.txt";
  std::ofstream reportEvaluation(reportEvaluationFile);

  //TODO moving average loss.

  const std::vector<std::string> categoryNames = m_dataset->get_category_names();

  // Get the detection layer (last layer).
  layer detectionLayer = net.layers[net.n - 1];

  std::cout << "net.batch: " << net.batch << '\n';
  std::cout << "net.subdivisions: " << net.subdivisions << '\n';


  const size_t imagesPerDatum = net.batch;
  const size_t numDatum = net.subdivisions;
  int imagesPerBatch = net.batch * net.subdivisions;

  size_t batchNumber = static_cast<size_t>(*net.seen/imagesPerBatch);

  // Set up the image paths.
  std::vector<std::string> trainPaths = m_dataset->get_image_paths(m_year, VOC_TRAIN, VOC_JPEG);
  //std::vector<std::string> trainPaths = boost::assign::list_of("/home/mikesapi/desktop/yolo/data/voc/VOCdevkit/VOC2012/JPEGImages/2011_003121.jpg");

  // TODO: Only in debug mode
  PerformanceTable table(boost::assign::list_of("trainPerf")("validPerf"));

  //const size_t epochCount(135); // in yolo paper; 1epoch corresponds to going through the training dataset once.
  const size_t imagesPerEpoch = trainPaths.size();
  const size_t maxImagesToShowNetwork(imagesPerEpoch * epochCount);
  const size_t maxBatchNumber = maxImagesToShowNetwork / imagesPerBatch;
  //const size_t maxBatchNumber = 40e3;

#if 0
  // Identity Transformation.
  DataTransformationFactory::Settings settings;
  settings.maxRotation = 0.0f;
  settings.maxTranslation = 0.0f;
  settings.maxSpatialScalePercentage = 0.0f;
  settings.useRandomFlipping = false;
  settings.maxScaleIntensityPercentage = 0.0f;
  settings.maxAddToIntensityValue = 0.0f;

  TrainingDataGenerator dataGenerator(trainPaths, m_dataset, net.w, net.h, settings, m_seed);
#else
  TrainingDataGenerator dataGenerator(trainPaths, m_dataset, net.w, net.h, DataTransformationFactory::Settings(), m_seed, m_shapeDescriptorCalculator);
#endif

  CQ_Ptr dataBuffer(new CircularQueue<std::vector<Datum> >(35));
  boost::thread dataLoadingThread(&TrainingDataGenerator::run_load_loop, boost::ref(dataGenerator), dataBuffer, batchNumber, maxBatchNumber, numDatum, imagesPerDatum, detectionLayer.jitter, m_ds);

  // Before starting to train, write a report with the settings used to train.
  const std::string saveSettingsFile(saveResultsDir + "/settings.txt");
  if(!boost::filesystem::exists(saveSettingsFile))
  {
    std::ofstream settingsFile(saveSettingsFile);
    settingsFile << m_ds << '\n';
    settingsFile << m_experimentUniqueStamp << '\n';
    settingsFile << dataGenerator << '\n';
  }

  MovingAverage<float> lossMovingAverage(20,0.0f);

  while(batchNumber < maxBatchNumber)
  {
    tvgutil::Timer<boost::chrono::milliseconds> batchProcessTime("batchProcessTime");

    ++batchNumber;
    float epoch = (batchNumber * imagesPerBatch) / static_cast<float>(imagesPerEpoch);

    float loss;
    while(true)
    {
      boost::optional<const std::vector<Datum>& > data = dataBuffer->pop();
      //boost::optional<std::vector<Datum> > data = dataGenerator.generate_training_data(numDatum, imagesPerDatum, detectionLayer.jitter, m_ds);

      if(data)
      {
#ifdef DEBUG_DATA
        for(size_t i = 0; i < numDatum; ++i)
        {
          dataGenerator.debug_training_datum(data[i], imagesPerDatum, m_ds);
          cv::waitKey(100);
        }
#endif
        std::vector<Datum>& data2 = const_cast<std::vector<Datum>& >(*data);
        TIME(loss = lossMovingAverage.push(train_network(net, data2));, milliseconds, trainTimeA);
        if(m_debugFlag) std::cout << trainTimeA << '\n';
        break;
      }
    }

    // Reporting
    std::string info("BatchNo: " + (sixDigits % batchNumber).str() + '/' + (sixDigits % maxBatchNumber).str()
              + ", Epoch: "      + (twoDecimalPlaces % epoch).str()
              + ", Loss: "       + (sixDecimalPlaces % loss).str()
              + ", Rate: "       + (sixDecimalPlaces % get_current_rate(net)).str()
              + ", NImages: "    + (tenDigits % (batchNumber * imagesPerBatch)).str() + '\n');
    reportLoss.open(reportLossFile, std::fstream::app);
    reportLoss << info;
    reportLoss.close();

    if(m_debugFlag) std::cout << info;

    // Save intermediate models.
    const bool saveIntermediateWeightsFlag(true);
    const int saveWeightsBatchInterval(10*(imagesPerEpoch/imagesPerBatch)); // Save a snapshot every 10 epochs
    std::string relativeIntermediateModelsDir = m_experimentUniqueStamp + "/intermediate-models";
    if((batchNumber > 1) && ((batchNumber % saveWeightsBatchInterval) == 0) && saveIntermediateWeightsFlag)
    {
      std::string tmpSaveWeightsFile = m_dataset->get_dir_in_results(relativeIntermediateModelsDir) + '/' + TimeUtil::get_iso_timestamp() + '-' + boost::lexical_cast<std::string>(batchNumber) + ".weights";
      save_weights(net, const_cast<char*>(tmpSaveWeightsFile.c_str()));
    }

    if(m_debugFlag)
    {
      const int evaluateEpochInterval(30*imagesPerEpoch/imagesPerBatch); // Evaluate learned model every 10 epochs
      if((batchNumber > 1) && ((batchNumber % evaluateEpochInterval) == 0))
      {
        evaluate_network(net, batchNumber, epoch, table, reportEvaluation, reportEvaluationFile);
      }
    }

    batchProcessTime.stop();
    std::cout << "###" << batchProcessTime << '\n' << std::endl;
  }

  // Create a table for plotting.
  std::string tableFile = saveResultsDir + "/table.txt";
  std::ofstream ofs(tableFile);
  table.output(ofs, " ");

  // Get the path to save the final weights file.
  std::string saveWeightsFile = saveResultsDir + '/' + m_experimentUniqueStamp + "-final.weights";

  save_weights(net, const_cast<char*>(saveWeightsFile.c_str()));
}

//#################### PRIVATE MEMBER FUNCTIONS ####################

float Trainer::get_and_save_performance(network& net, VOCYear vocYear, VOCSplit vocSplit, size_t batchNumber) const
{
  std::string stamp = m_dataset->get_split_name(vocSplit) + TimeUtil::get_iso_timestamp();
  std::string saveResultsPathTrain = m_dataset->get_dir_in_results(m_experimentUniqueStamp + '/' + "intermediate-results" + '/' + stamp);
  Evaluator evaluator(m_dataset, m_ds, m_shapeDescriptorCalculator);
  double overlapThreshold(0.5);
  return evaluator.calculate_map(net, saveResultsPathTrain, vocYear, vocSplit, "batchNumber-" + boost::lexical_cast<std::string>(batchNumber), overlapThreshold, m_maxImagesToEvaluateOn);
}

void Trainer::evaluate_network(network& net, size_t batchNumber, float epoch, PerformanceTable& table, std::ofstream& reportEvaluation, const std::string& reportEvaluationFile) const
{
  // Get the performance on the training set.
  float trainPerf = get_and_save_performance(net, m_year, VOC_TRAIN, batchNumber);
  float validPerf = get_and_save_performance(net, m_year, VOC_VAL, batchNumber);

  // Create a result and save it to a table.
  boost::format sixDigits("%06d");
  boost::format sixDecimalPlaces("%0.6f");
  ParamSet params = boost::assign::map_list_of
    ("batchNumber",(sixDigits % batchNumber).str())
    ("learningRate",(sixDecimalPlaces % net.learning_rate).str());

  std::map<std::string,PerformanceMeasure> result = boost::assign::map_list_of
    ("trainPerf",PerformanceMeasure(trainPerf))
    ("validPerf",PerformanceMeasure(validPerf));

  table.record_performance(params, result);

  reportEvaluation.open(reportEvaluationFile, std::fstream::app);
  reportEvaluation << "\n\n~~~~~~~~~~~~~~~~~~~~ DEBUG TRAIN ~~~~~~~~~~~~~~~~~~~~\n";
  reportEvaluation << "epoch=" << (sixDecimalPlaces % epoch).str() << '\n';
  reportEvaluation << "trainMAP=" << trainPerf << '\n';
  reportEvaluation << "valMAP=" << validPerf << '\n';
  reportEvaluation << "batchNumber=" << batchNumber << '\n';
  reportEvaluation << "~~~~~~~~~~~~~~~~~~~~ DEBUG TRAIN ~~~~~~~~~~~~~~~~~~~~\n\n";
  reportEvaluation.close();
}

float Trainer::train_network(network& net, Datum& datum) const
{
  const int miniBatchSize = net.batch; //This is the batchSize/subdivisions.
  float error = train_network_datum(net, datum.first.data(), datum.second.data());
  return error/miniBatchSize;
}

float Trainer::train_network(network& net, std::vector<Datum>& data) const
{
  float sum(0.0f);
  const size_t numDatum = data.size();
  for(size_t i = 0; i < numDatum; ++i)
  {
    float error = train_network(net, data[i]);
    sum += error;
  }
  return sum/static_cast<float>(numDatum);
}


