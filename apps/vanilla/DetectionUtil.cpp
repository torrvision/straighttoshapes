/**
 * vanilla: DetectionUtil.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016, All rights reserved.
 */

#include "core/DetectionComparator.h"
#include "data/InputDataAssembler.h"
#include "DetectionUtil.h"
#include "Util.h"

#include "DarknetUtil.h"

#include "core/Box.h"

#include <cmath>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <tvgutil/misc/ArgUtil.h>
#include <tvgutil/containers/MapUtil.h>
using namespace tvgutil;
using namespace tvgshape;

#include <tvgplot/PaletteGenerator.h>

typedef boost::shared_ptr<CircularQueue<std::pair<cv::Mat3b,cv::Size> > > CQ_Ptr;

std::vector<Detections> DetectionUtil::detect(network& net, const std::vector<std::string>& paths, const DetectionSettings& ds, const boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator)
{
  size_t pathCount = paths.size();
  std::vector<Detections> detections(pathCount);
  for(size_t i = 0; i < pathCount; ++i)
  {
    if(i % 50 == 0) std::cout << i << ' ' << std::flush;
    if(i % 500 == 0) std::cout << std::endl;

    detections[i] = detect(net, paths[i], ds, shapeDescriptorCalculator);
  }

  return detections;
}

std::vector<Detections> DetectionUtil::detect_fast(network& net, const std::vector<std::string>& paths, const DetectionSettings& ds, const boost::optional<ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator)
{
  size_t pathCount = paths.size();
  std::vector<Detections> detections(pathCount);

  //TODO: only store shared pointers to types in the circular queue.
  CQ_Ptr dataBuffer(new CircularQueue<std::pair<cv::Mat3b,cv::Size> >(10));
  InputDataAssembler dataLoader(paths, dataBuffer, net.w, net.h);
  boost::thread dataLoadingThread(&InputDataAssembler::run_load_loop, boost::ref(dataLoader));

  for(size_t i = 0; i < pathCount; ++i)
  {
    if(i > 0)
    {
      if(i % 50 == 0) std::cout << i << ' ' << std::flush;
      if(i % 500 == 0) std::cout << std::endl;
    }

    while(true)
    {
      boost::optional<const std::pair<cv::Mat3b,cv::Size>& > data = dataBuffer->pop();
      if(data)
      {
        detections[i] = detect(net, (*data).first, (*data).second.width, (*data).second.height, ds, shapeDescriptorCalculator);
        break;
      }
    }
  }
  std::cout << '\n' << std::endl;

  return detections;
}

Detections DetectionUtil::detect(network& net, const std::string& path, const DetectionSettings& ds, const boost::optional<ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator)
{
  cv::Mat3b im = cv::imread(path, CV_LOAD_IMAGE_COLOR);
  Detections detections = detect(net, im, im.cols, im.rows, ds);

  return detections;
}

Detections DetectionUtil::detect(network& net, const cv::Mat3b& image, int originalImageWidth, int originalImageHeight, const DetectionSettings& ds, const boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator)
{
  std::vector<float> predictions = get_raw_predictions(net, image, originalImageWidth, originalImageHeight);

  Detections d = DetectionUtil::extract_detections(predictions, ds, originalImageWidth, originalImageHeight, shapeDescriptorCalculator);

  if(ds.nms)
  {
    d = DetectionUtil::non_maximal_suppression(d, ds.overlapThreshold);
  }

  d = DetectionUtil::prune_detections(d, ds.detectionThreshold);

  return d;
  /*
  if(ds.nms) return DetectionUtil::non_maximal_suppression(d, ds.overlapThreshold);
  else return d;
  */
}

Detections DetectionUtil::prune_detections(const Detections& detections, float detectionThreshold)
{
  Detections pruned;

  for(size_t i = 0; i < detections.size(); ++i)
  {
    size_t category = ArgUtil::argmax(detections[i].second);
    float score = detections[i].second[category];
    if(score >= detectionThreshold)
    {
      pruned.push_back(detections[i]);
    }
  }

  return pruned;
}

std::vector<float> DetectionUtil::get_raw_predictions(network& net, const cv::Mat3b& image, int originalImageWidth, int originalImageHeight)
{
  const int imageWidthNetwork = net.w;
  const int imageHeightNetwork = net.h;

  std::vector<float> predictions;
  if(image.cols != imageWidthNetwork || image.rows != imageHeightNetwork)
  {
    if(image.cols != originalImageWidth || image.rows != originalImageHeight)
    {
      throw std::runtime_error("The original image width and the input image width should be the same");
    }
    cv::Mat3b resizedImage(net.w, net.h);
    cv::resize(image, resizedImage, cv::Size(imageWidthNetwork, imageHeightNetwork));
    predictions = DarknetUtil::predict(net, resizedImage);
  }
  else
  {
    // Assume that the image has been resized externally.
    predictions = DarknetUtil::predict(net, image);
  }

  return predictions;
}

Detections DetectionUtil::extract_detections(const std::vector<float>& predictions, const DetectionSettings& ds, size_t inputImageWidth, size_t inputImageHeight, const boost::optional<ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator)
{
  const size_t categoryCount = ds.categoryCount;
  const size_t gridSide = ds.gridSideLength;
  const size_t boxesPerCell = ds.boxesPerCell;

  size_t cellCount = gridSide * gridSide;
  Detections detections(boxesPerCell * cellCount);
  for(size_t i = 0; i < cellCount; ++i)
  {
    // Get the row and column index, cells are in row major format.
    size_t row = i / gridSide;
    size_t col = i % gridSide;

    // Prediction array:
    // [ (.. prob mass fns ..)(.. box confidences ..)(.. boxes ..) ]
    // [ ( pmf cell1<pmf1> )( conf cell1<conf1,conf2..> cell2<..> )( boxes cell1<box1,box2..> cell2<..>)
    // [ ( cellCount * categoryCount )( cellCount * boxesPerCell )( cellCount * (paramsPerBox + paramsPerShape) * boxesPerCell) ]
    for(size_t b = 0; b < boxesPerCell; ++b)
    {
      // Index into the detection array.
      size_t arrayIndex = i * boxesPerCell + b;

      size_t confIndex = cellCount*categoryCount + i * boxesPerCell + b;
      // The confidence prediction represents the IOU between the predicted box and any ground truth box.
      float boxConfidence = predictions[confIndex];

      // If there are nonsensical values in the prediction, then just output a dummy detection filled with zeros.
      if(std::isnan(boxConfidence))
      {
        Shape shape(VOCBox(0,0,0,0));
        detections[arrayIndex] = std::make_pair(shape, std::vector<float>(categoryCount, 0.0f));
        continue;
      }

      size_t boxIndex = cellCount * (categoryCount + boxesPerCell) + (i * boxesPerCell + b) * (ds.paramsPerBox + ds.paramsPerShapeEncoding);
      float relativex = predictions[boxIndex + 0];
      float relativey = predictions[boxIndex + 1];
      float w_ = predictions[boxIndex + 2];
      float h_ = predictions[boxIndex + 3];

      float x = ((relativex + col) / gridSide) * inputImageWidth;
      float y = ((relativey + row) / gridSide) * inputImageHeight;
      float w = (ds.useSquare ? w_*w_ : w_) * inputImageWidth;
      float h = (ds.useSquare ? h_*h_ : h_) * inputImageHeight;

      Box box(x < 0.0f ? 0.0f : x, y < 0.0f ? 0.0f : y, w < 0.0f ? 0.0f : w, h < 0.0f ? 0.0f : h);
      VOCBox vbox(box);

      // Make sure that the box is within the bounds of the image.
      // FIXME: make sure that clipping the box has a corresponding effect on the shape mask.
      vbox.clip_to_image_boundaries(inputImageWidth, inputImageHeight);

      if(!vbox.valid())
      {
        Shape shape(VOCBox(0,0,0,0));
        detections[arrayIndex] = std::make_pair(shape, std::vector<float>(categoryCount, 0.0f));
        continue;
      }

      std::vector<float> classConfidenceScores(categoryCount);
      float maxConfidence(0.0f);
      for(size_t c = 0; c < categoryCount; ++c)
      {
        size_t categoryIndex = i * categoryCount;
        float conditionalProbability = predictions[categoryIndex + c];
        float classProbability = boxConfidence * conditionalProbability;
        classConfidenceScores[c] = (classProbability > ds.detectionThreshold) ? classProbability : 0.0f;
        if(maxConfidence < classConfidenceScores[c])
        {
          maxConfidence = classConfidenceScores[c];
        }
      }

      cv::Mat1b mask;
      if((ds.paramsPerShapeEncoding > 0) && (maxConfidence >= ds.detectionThreshold)) // If shape is activated.
      {
        const float *shapeData = &predictions[boxIndex + ds.paramsPerBox];

        if(shapeDescriptorCalculator)
        {
          std::vector<float> encoding(shapeData, shapeData + ds.paramsPerShapeEncoding);
          mask = (*shapeDescriptorCalculator)->to_mask(encoding, cv::Size(vbox.w(), vbox.h()));
        }
        else throw std::runtime_error("Expecting a shape descriptor calculator!");
      }


      if(ds.onlyObjectness)
      {
        classConfidenceScores[0] = boxConfidence;
      }

      Shape shape(vbox, mask);
      detections[arrayIndex] = std::make_pair(shape, classConfidenceScores);
    }
  }

  return detections;
}

Detections DetectionUtil::non_maximal_suppression(const Detections& d, float overlapThreshold)
{
  if(d.empty()) return d;

  Detections nmsd(d);

  size_t detectionCount = nmsd.size();
  size_t categoryCount = nmsd[0].second.size();
  for(size_t c = 0; c < categoryCount; ++c)
  {
    DetectionComparator<Detection> comp(c, std::greater<float>());
    std::sort(nmsd.begin(), nmsd.end(), comp);

    for(size_t i = 0; i < detectionCount; ++i)
    {
      if(nmsd[i].second[c] < std::numeric_limits<float>::min()) continue;

      VOCBox b1 = nmsd[i].first.get_voc_box();
      for(size_t j = i + 1; j < detectionCount; ++j)
      {
        VOCBox b2 = nmsd[j].first.get_voc_box();
        if(b1.overlap(b2) > overlapThreshold)
        {
          nmsd[j].second[c] = -1.0f;
        }
      }
    }
  }

  return nmsd;
}

cv::Mat3b DetectionUtil::overlay_detections(const cv::Mat3b& im, const Detections& detections, float displayDetectionThreshold, const std::vector<std::string>& categoryNames, const std::map<size_t,cv::Scalar>& palette)
{
//#define DEBUG_OVERLAY_DETECTIONS
#ifdef DEBUG_OVERLAY_DETECTIONS
  size_t displayDetectionCount(0);
#endif

  cv::Mat3b displayImage = im.clone();

  for(size_t i = 0, detectionCount = detections.size(); i < detectionCount; ++i)
  {
    // Check whether the best score is below the detection threshold.
    std::vector<float> scores = detections[i].second;
    size_t category = ArgUtil::argmax(scores);
    float score = scores[category];
    if(score < displayDetectionThreshold) continue;

    // Draw the box with line thickness in proportion to the score.
    VOCBox b = detections[i].first.get_voc_box();
    int lineThickness = static_cast<int>(score * 8.0f);
    cv::Scalar colour = MapUtil::lookup(palette, category + 1); // FIXME: +1 since we are not modelling background class.
    cv::rectangle(displayImage, cv::Rect_<float>(b.tlx(), b.tly(), b.w(), b.h()), colour, lineThickness, 8, 0);
    cv::putText(displayImage, categoryNames[category], cv::Point2i(b.tlx() + 5, b.tly() + 15), cv::FONT_HERSHEY_SIMPLEX, 0.7, colour, 2);

    // If there is a shape mask associated with the bounding box, overlay it on the display image.
    cv::Mat1b mask = detections[i].first.get_mask();
    if(mask.data)
    {
      //Debugging
#ifdef DEBUG_OVERLAY_DETECTIONS
    cv::imshow("objectMask-" + categoryNames[category] + "s:" + boost::lexical_cast<std::string>(score) + " id:" + boost::lexical_cast<std::string>(displayDetectionCount++), mask);
#endif
      // Resize the shape mask to the bounding box.
      // FIXME duplicate code in Shape.cpp
      cv::Rect rect = Util::to_rect(b);
      cv::Mat1b resizedMask(rect.height, rect.width);
      if(mask.cols != rect.height || mask.rows != rect.height)
      {
        cv::resize(mask, resizedMask, resizedMask.size(), 0.0, 0.0, cv::INTER_CUBIC);
      }
      else
      {
        resizedMask = mask;
      }

      // Colourise the mask
      cv::Mat3b colourisedMask = Util::colourise_mask(resizedMask, colour/255.0f);

      // Add the mask to the display image.
      cv::Mat3b roiDisplayImage = displayImage(rect);
      cv::add(roiDisplayImage, colourisedMask, roiDisplayImage, resizedMask);
    }
  }

  return displayImage;
}

Detections DetectionUtil::voc_objects_to_detections(const std::vector<VOCObject>& objects, size_t categoryCount)
{
  Detections detections;

  const size_t numObjects = objects.size();
  for(size_t i = 0; i < numObjects; ++i)
  {
    const VOCObject& obj = objects[i];
    std::vector<float> scores = Util::generate_one_hot<float>(obj.categoryId, categoryCount);
    detections.push_back(std::make_pair(obj.rep, scores));
  }

  return detections;
}
