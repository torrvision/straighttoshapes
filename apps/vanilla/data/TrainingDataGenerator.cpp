/**
 * vanilla: TrainingDataGenerator.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "TrainingDataGenerator.h"

#include "../Util.h"

#include "../core/Box.h"

#include "../DetectionUtil.h"

#include <boost/bind/bind.hpp>
#include <boost/lexical_cast.hpp>

#ifdef WITH_OPENMP
#include <omp.h>
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <tvgutil/timing/Timer.h>
#include <tvgutil/misc/ArgUtil.h>
#include <tvgutil/statistics/ProbabilityMassFunction.h>
using namespace tvgutil;
using namespace tvgshape;

#if 0
std::vector<float> generate_random_vector(tvgutil::RandomNumberGenerator& rng, size_t size)
{
  std::vector<float> result(size);
  for(size_t i = 0; i < size; ++i)
  {
    result[i] = rng.generate_from_gaussian(0.0f, 0.5f);
  }
  return result;
}
#endif

TrainingDataGenerator::TrainingDataGenerator(const std::vector<std::string>& imagePaths, const Dataset_CPtr& dataset, size_t imageWidthNetwork, size_t imageHeightNetwork, const DataTransformationFactory::Settings& dataTransformationSettings, unsigned int seed, const boost::optional<ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator)
: m_dataset(dataset),
  m_dataTransformationFactory(seed, dataTransformationSettings),
  m_imageHeightNetwork(imageHeightNetwork),
  m_imagePaths(imagePaths),
  m_imageWidthNetwork(imageWidthNetwork),
  m_rng(seed),
  m_shapeDescriptorCalculator(shapeDescriptorCalculator)
{}

void TrainingDataGenerator::run_load_loop(CQ_Ptr& cq, size_t startBatchNumber, size_t maxBatchNumber, size_t numDatum, size_t imagesPerDatum, float jitter, const DetectionSettings& ds) const
{
  for(size_t i = startBatchNumber; i < maxBatchNumber; ++i)
  {
    //TIME(
    std::vector<Datum> data = generate_training_data(numDatum, imagesPerDatum, jitter, ds);
    //, milliseconds, dataLoadTime); std::cout << dataLoadTime << '\n';

    bool loaded = false;
    while(!loaded)
    {
      loaded = cq->push(data);
    }
  }
}

std::vector<Datum> TrainingDataGenerator::generate_training_data(size_t numDatum, size_t imagesPerDatum, float jitter, const DetectionSettings& ds) const
{
  std::vector<Datum> data(numDatum);
#ifdef WITH_OPENMP
  #pragma omp parallel for
#endif
  for(int i = 0; i < static_cast<int>(numDatum); ++i)
  {
    data[i] = generate_training_datum(imagesPerDatum, jitter, ds);
  }

  return data;
}

Datum TrainingDataGenerator::generate_training_datum(size_t imagesPerDatum, float jitter, const DetectionSettings& ds) const
{
  // Get random paths.
  std::vector<std::string> randomPaths = generate_random_image_paths(imagesPerDatum);
  std::vector<DataTransformation> dataTransformations = m_dataTransformationFactory.generate_transformations(randomPaths.size());
  std::vector<float> input = prepare_input(randomPaths, dataTransformations);
  //TIME(
  std::vector<float> target = prepare_target(randomPaths, ds, dataTransformations);
  //, milliseconds, prepareTarget); std::cout << prepareTarget << '\n';

  // TODO
  std::cout << m_hist << std::endl;

  return std::make_pair(input, target);
}

std::vector<std::string> TrainingDataGenerator::generate_random_image_paths(size_t imagesPerDatum) const
{

  size_t multiplier(1);

#if 1
  multiplier = 3;
#endif
  const size_t pathCount(m_imagePaths.size());
  std::vector<std::string> randomPaths;
  for(size_t i = 0; i < imagesPerDatum*multiplier; ++i)
  {
    randomPaths.push_back(m_imagePaths[m_rng.generate_int_from_uniform(0,pathCount - 1)]);
  }

  std::vector<std::string> bestPaths;
#if 1
  bestPaths = pick_paths(randomPaths, imagesPerDatum);
#else
  bestPaths = randomPaths;
#endif

  for(size_t i = 0; i < bestPaths.size(); ++i)
  {
    const std::vector<VOCObject>& objects = m_dataset->get_annotation_from_image_path(bestPaths[i])->get_objects();
    for(size_t i = 0; i < objects.size(); ++i)
    {
      m_hist.add(objects[i].categoryName);
    }
  }

  return bestPaths;
}

std::vector<std::string> TrainingDataGenerator::pick_paths(const std::vector<std::string>& randomPaths, size_t maximumSize) const
{
  std::vector<std::string> bestPaths;

  if(!m_hist.empty())
  {
    std::vector<std::pair<size_t,double> > scores;

    ProbabilityMassFunction<std::string> before(m_hist);
    double beforeEntropy = before.calculate_entropy();

    // Accumulate the category statistics.
    for(size_t i = 0; i < randomPaths.size(); ++i)
    {
      const std::vector<VOCObject>& objects = m_dataset->get_annotation_from_image_path(randomPaths[i])->get_objects();

      Histogram<std::string> objectHist;
      for(size_t i = 0; i < objects.size(); ++i)
      {
        objectHist.add(objects[i].categoryName);
      }

      Histogram<std::string> hist = histogram_add(m_hist, objectHist);
      ProbabilityMassFunction<std::string> after(hist);

      // smaller if better
      double score = beforeEntropy - after.calculate_entropy();

      scores.push_back(std::make_pair(i,score));
    }

    std::sort(scores.begin(), scores.end(), boost::bind(&std::pair<size_t,double>::second, _1) <
                                            boost::bind(&std::pair<size_t,double>::second, _2));

    for(size_t i = 0; i < maximumSize; ++i)
    {
      std::string path = randomPaths[scores[i].first];
      bestPaths.push_back(path);
    }
  }
  else
  {
    bestPaths = randomPaths;
    bestPaths.erase(bestPaths.begin() + maximumSize, bestPaths.end());
  }

  return bestPaths;
}

Histogram<std::string> TrainingDataGenerator::histogram_add(const Histogram<std::string>& a, const Histogram<std::string>& b) const
{
  Histogram<std::string> result(a);
  const std::map<std::string,size_t>& bins = b.get_bins();
  for(std::map<std::string,size_t>::const_iterator it = bins.begin(), iend = bins.end(); it != iend; ++it)
  {
    std::string category = it->first;
    size_t count = it->second;
    for(size_t i = 0; i < count; ++i)
    {
      result.add(category);
    }
  }

  return result;
}

std::vector<float> TrainingDataGenerator::prepare_input(const std::vector<std::string>& imagePaths, const std::vector<DataTransformation>& dataTransformations) const
{
#if 1
  const size_t imagesPerDatum = imagePaths.size();
  const size_t pixelsPerImage = m_imageWidthNetwork * m_imageHeightNetwork * 3;
  const size_t inputSize = imagesPerDatum * pixelsPerImage;
  std::vector<float> input(inputSize);
  for(size_t i = 0; i < imagesPerDatum; ++i)
  {
    if(!boost::filesystem::exists(imagePaths[i])) throw std::runtime_error("The path " + imagePaths[i] + " does not exist");
    // Read in the input image.
    cv::Mat3b im = cv::imread(imagePaths[i], CV_LOAD_IMAGE_COLOR);

    // Apply a data transformation.
    cv::Mat3b imt = dataTransformations[i].apply_real_image_transformation(im);

    float *imDarknetFormat = Util::make_rgb_image(imt, 1/255.0f);
    std::copy(imDarknetFormat, imDarknetFormat + pixelsPerImage, input.begin() + i * pixelsPerImage);
    delete [] imDarknetFormat;
  }
#else
  const size_t imagesPerDatum = imagePaths.size();
  const size_t pixelsPerImage = 10;
  const size_t inputSize = imagesPerDatum * pixelsPerImage;
  std::vector<float> input(inputSize);
  for(size_t i = 0; i < imagesPerDatum; ++i)
  {
    std::vector<float> im(pixelsPerImage,i);
    std::copy(im.begin(), im.begin() + pixelsPerImage, input.begin() + i * pixelsPerImage);
  }
#endif
  return input;
}

std::vector<float> TrainingDataGenerator::prepare_target(const std::vector<std::string>& imagePaths, const DetectionSettings& ds, const std::vector<DataTransformation>& dataTransformations) const
{
#if 1
  const size_t paramsPerCell = ds.paramsPerConfidenceScore
                             + ds.paramsPerBox
                             + ds.paramsPerShapeEncoding
                             + ds.categoryCount;

  const size_t imagesPerDatum = imagePaths.size();
  const size_t predictedVariablesPerImage = ds.gridSideLength * ds.gridSideLength * paramsPerCell;
  const size_t targetSize = imagesPerDatum * predictedVariablesPerImage;
  std::vector<float> target(targetSize);
  for(size_t i = 0; i < imagesPerDatum; ++i)
  {
    //TIME(
    Detections transformedDetections = m_dataset->get_detections_from_image_path(imagePaths[i], dataTransformations[i]);
    //, milliseconds, getDetectionsFromImagePath); std::cout << getDetectionsFromImagePath << '\n';

    std::vector<float> singleTarget = create_target_from_detections(transformedDetections, ds);
    std::copy(singleTarget.begin(), singleTarget.begin() + predictedVariablesPerImage, target.begin() + i * predictedVariablesPerImage);
  }
#else
  const size_t imagesPerDatum = imagePaths.size();
  const size_t predictedVariablesPerImage = 5;
  const size_t targetSize = imagesPerDatum * predictedVariablesPerImage;
  std::vector<float> target(targetSize);
  for(size_t i = 0; i < imagesPerDatum; ++i)
  {
    std::vector<float> var(predictedVariablesPerImage, 10*i);
    std::copy(var.begin(), var.begin() + predictedVariablesPerImage, target.begin() + i * predictedVariablesPerImage);
  }
#endif
  return target;
}

//store the ground truth in the following format:
//0,0:[(conf)(gt prob mass fn)(box)]
//0,1:[(conf)(gt prob mass fn)(box)]
//...
//7,7:[(conf)(gt prob mass fn)(box)]
std::vector<float> TrainingDataGenerator::create_target_from_detections(const Detections& detections, const DetectionSettings& ds) const
{
  const size_t paramsPerCell = ds.paramsPerConfidenceScore
                             + ds.paramsPerBox
                             + ds.paramsPerShapeEncoding
                             + ds.categoryCount;

  const size_t targetSize = ds.gridSideLength * ds.gridSideLength * paramsPerCell;
  std::vector<float> target(targetSize, 0.0f);

  for(size_t i = 0, numDetections = detections.size(); i < numDetections; ++i)
  {
    VOCBox vbox = detections[i].first.get_voc_box();
    vbox.clip_to_image_boundaries(m_imageWidthNetwork, m_imageHeightNetwork);
    if(!vbox.valid()) continue;

    Box box(vbox);
    // Transform the box corrdinates to be between zero and 1
    box.scale(1.0f/(float)m_imageWidthNetwork, 1.0f/(float)m_imageHeightNetwork);
    // Transform the box coordinates to be realtive to the grid cell in which it lies.

    // Find box centre relative to grid.
    int col = static_cast<int>(box.x*ds.gridSideLength);
    int row = static_cast<int>(box.y*ds.gridSideLength);

    box.x = box.x*ds.gridSideLength - col; // only keep relative offset to cell.
    box.y = box.y*ds.gridSideLength - row;

    const std::vector<float>& pmf = detections[i].second;
    const float boxConfidence = 1.0f;

    int index = (col + row * ds.gridSideLength) * paramsPerCell;
    if(target[index] > std::numeric_limits<float>::min()) continue;
    target[index++] = boxConfidence;
    std::copy(pmf.begin(), pmf.end(), target.begin() + index);
    index += ds.categoryCount;
    target[index++] = box.x;
    target[index++] = box.y;
    target[index++] = box.w;
    target[index++] = box.h;

    if(ds.paramsPerShapeEncoding > 0)
    {
      cv::Mat1b mask = detections[i].first.get_mask();
      if(mask.data)
      {
        if(m_shapeDescriptorCalculator)
        {
          //TIME(
          std::vector<float> encoding = (*m_shapeDescriptorCalculator)->from_mask(mask, ds.paramsPerShapeEncoding);
          //, milliseconds, fromMask); std::cout << fromMask << std::endl;
          std::copy(encoding.data(), encoding.data() + ds.paramsPerShapeEncoding, target.begin() + index);
        }
        else throw std::runtime_error("Expecting a shape descriptor calculator!");
      }
    }
  }

  return target;
}

Detections TrainingDataGenerator::create_detections_from_target(const std::vector<float>& target, const DetectionSettings& ds) const
{
  const size_t paramsPerCell = ds.paramsPerConfidenceScore
                             + ds.paramsPerBox
                             + ds.paramsPerShapeEncoding
                             + ds.categoryCount;

  Detections detections;
  const size_t numCells = ds.gridSideLength*ds.gridSideLength;
  for(size_t i = 0; i < numCells; ++i)
  {
    size_t detectionIndex = i * paramsPerCell;
    detectionIndex++;//const float boxConfidence = target[detectionIndex++];
    std::vector<float> pmf(ds.categoryCount);
    std::copy(target.begin() + detectionIndex, target.begin() + detectionIndex + ds.categoryCount, pmf.begin());

    size_t category = ArgUtil::argmax(pmf);
    float score = pmf[category];
    if(score < ds.detectionThreshold) continue;

    detectionIndex += ds.categoryCount;
    float x = target[detectionIndex++];
    float y = target[detectionIndex++];
    float w = target[detectionIndex++];
    float h = target[detectionIndex++];
    Box box(x, y, w, h);

    size_t row = i / ds.gridSideLength;
    size_t col = i % ds.gridSideLength;

    box.x = (box.x + col)/ds.gridSideLength;
    box.y = (box.y + row)/ds.gridSideLength;

    box.scale(m_imageWidthNetwork, m_imageHeightNetwork);

    VOCBox vbox(box);

    cv::Mat1b mask;
    if(ds.paramsPerShapeEncoding > 0)
    {
      if(m_shapeDescriptorCalculator)
      {
        std::vector<float> encoding(&target[detectionIndex], &target[detectionIndex] + ds.paramsPerShapeEncoding);
        //TIME(
        mask = (*m_shapeDescriptorCalculator)->to_mask(encoding, cv::Size(vbox.w(), vbox.h()));
        //, milliseconds, toMask); std::cout << toMask << std::endl;
      }
      else throw std::runtime_error("Expecting a shape descriptor calculator!");
    }

    Shape shape(vbox, mask);
    detections.push_back(std::make_pair(shape, pmf));
  }

  return detections;
}

void TrainingDataGenerator::debug_training_datum(const Datum& datum, size_t imagesPerDatum, const DetectionSettings& ds) const
{
  const size_t paramsPerCell = ds.paramsPerConfidenceScore
                             + ds.paramsPerBox
                             + ds.paramsPerShapeEncoding
                             + ds.categoryCount;

  const size_t pixelsPerImage = m_imageWidthNetwork * m_imageHeightNetwork * 3;
  const size_t inputSize = imagesPerDatum * pixelsPerImage;
  std::vector<float> input = datum.first;
  if(input.size() != inputSize) throw std::runtime_error("size does not match input");

  const size_t singleTargetSize = ds.gridSideLength * ds.gridSideLength * paramsPerCell;
  const size_t targetSize = imagesPerDatum * singleTargetSize;
  std::vector<float> target = datum.second;
  if(target.size() != targetSize) throw std::runtime_error("size does not match target");

  std::vector<std::string> windowNames;
  for(size_t i = 0; i < imagesPerDatum; ++i)
  {
    windowNames.push_back("indexInBatch" + boost::lexical_cast<std::string>(i));

    // Construct input.
    std::vector<float>::iterator start = input.begin() + (pixelsPerImage * i);
    std::vector<float> darknetImage(start, start + pixelsPerImage);
    cv::Mat3b im = Util::make_rgb_image(darknetImage.data(), m_imageWidthNetwork, m_imageHeightNetwork, 255.0f);

    // Construct target.
    start = target.begin() + (singleTargetSize * i );
    std::vector<float> singleTarget(start, start + singleTargetSize);
    Detections detections = create_detections_from_target(singleTarget, ds);

    cv::Mat3b displayImage = DetectionUtil::overlay_detections(im, detections, ds.detectionThreshold, m_dataset->get_category_names(), m_dataset->get_palette());

    // Display.
    cv::namedWindow(windowNames[i], cv::WINDOW_AUTOSIZE);
    cv::imshow(windowNames[i], displayImage);
  }

  cv::waitKey();
  cv::destroyAllWindows();
}

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const TrainingDataGenerator& d)
{
  os << d.m_dataset << '\n';
  os << d.m_dataTransformationFactory << '\n';
  return os;
}
