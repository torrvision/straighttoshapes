/**
 * vanilla: Evaluator.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016, All rights reserved.
 */

#include "Evaluator.h"
#include "DetectionUtil.h"
#include "Util.h"

#include "core/Detection.h"
#include "core/TupleComparator.h"

#include <numeric>

#include <boost/assign/list_of.hpp>
#include <boost/bind/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
using namespace boost::assign;

#include <evaluation/core/PerformanceMeasure.h>
#include <evaluation/core/PerformanceTable.h>
using namespace evaluation;

#include <tvgutil/persistence/LineUtil.h>
#include <tvgutil/timing/Timer.h>
#include <tvgutil/persistence/LineUtil.h>
#include <tvgutil/misc/ArgUtil.h>
using namespace tvgutil;
using namespace tvgshape;

//#################### CONSTRUCTORS ####################

Evaluator::Evaluator(const Dataset_CPtr& dataset, const DetectionSettings& ds, const boost::optional<tvgshape::ShapeDescriptorCalculator_CPtr>& shapeDescriptorCalculator)
: m_dataset(dataset),
  m_ds(ds),
  m_shapeDescriptorCalculator(shapeDescriptorCalculator)
{}

//#################### PUBLIC MEMBER FUNCTIONS ####################

double Evaluator::calculate_map(network& net, const std::string& saveResultsPath, VOCYear vocYear, VOCSplit vocSplit, const std::string& uniqueStamp, double overlapThreshold, const boost::optional<size_t>& maxImages) const
{
  write_settings_report(saveResultsPath, uniqueStamp, vocYear, vocSplit);

  std::vector<std::string> imagePaths= m_dataset->get_image_paths(vocYear, vocSplit, VOC_JPEG, maxImages);

  std::cout << "\nCalculating detections..\n" << std::endl;
  std::vector<NamedCategoryDetections> perClassNamedCategoryDetections = calculate_detections_per_category(net, imagePaths);

  std::cout << "\nPreparing the ground truth..\n" << std::endl;
  NameToVOCObjectsHash nameToVOCObjectsHash = create_name_to_objects_hash(imagePaths);

  return calculate_map(perClassNamedCategoryDetections, nameToVOCObjectsHash, saveResultsPath, uniqueStamp, overlapThreshold);
}

double Evaluator::calculate_map_vol(network& net, const std::string& saveResultsPath, VOCYear vocYear, VOCSplit vocSplit, const std::string& uniqueStamp, const std::vector<double>& overlapThresholds, const boost::optional<size_t>& maxImages) const
{
  write_settings_report(saveResultsPath, uniqueStamp, vocYear, vocSplit);

  std::vector<std::string> imagePaths= m_dataset->get_image_paths(vocYear, vocSplit, VOC_JPEG, maxImages);

  std::cout << "\nCalculating detections..\n" << std::endl;
  std::vector<NamedCategoryDetections> perClassNamedCategoryDetections = calculate_detections_per_category(net, imagePaths);

  std::cout << "\nPreparing the ground truth..\n" << std::endl;
  NameToVOCObjectsHash nameToVOCObjectsHash = create_name_to_objects_hash(imagePaths);

  std::vector<double> mapPerThreshold(overlapThresholds.size());
  for(size_t i = 0; i < overlapThresholds.size(); ++i)
  {
    std::cout << "\nCalculating map with overlap threshold: " << overlapThresholds[i] << std::endl;
    mapPerThreshold[i] = calculate_map(perClassNamedCategoryDetections, nameToVOCObjectsHash, saveResultsPath, uniqueStamp, overlapThresholds[i]);
  }

  boost::format tenDecimalPlaces("%0.10f");
  double mapVol = Util::average_vector(mapPerThreshold);
  const std::string saveResultFile(saveResultsPath + "/mapVol.txt");
  std::ofstream resultsFile(saveResultFile);
  resultsFile << (tenDecimalPlaces % mapVol).str();

  return mapVol;
}

void Evaluator::find_save_best_worst(network& net, const std::string& saveResultsPath, VOCYear year, VOCSplit split) const
{
  std::vector<std::string> imagePaths= m_dataset->get_image_paths(year, split, VOC_JPEG);

  std::cout << "\nCalculating detections..\n" << std::endl;
  TIME(
  std::vector<Detections> detections = DetectionUtil::detect_fast(net, imagePaths, m_ds, m_shapeDescriptorCalculator);
  , seconds, detectionCalculationTime); std::cout << detectionCalculationTime;

  if(detections.size() != imagePaths.size()) throw std::runtime_error("sizes must be equal");

  std::cout << "\nPreparing the ground truth..\n" << std::endl;
  NameToVOCObjectsHash nameToVOCObjectsHash = create_name_to_objects_hash(imagePaths);

  std::cout << "\nSorting..\n" << std::endl;
  std::vector<std::pair<size_t,double> > scores;
  for(size_t i = 0; i < imagePaths.size(); ++i)
  {
    double score = get_score_per_image(nameToVOCObjectsHash, imagePaths[i], detections[i]);
    scores.push_back(std::make_pair(i, score));
  }

  std::sort(scores.begin(), scores.end(), boost::bind(&std::pair<size_t,double>::second, _1) >
                                          boost::bind(&std::pair<size_t,double>::second, _2));

  boost::format threeDecimalPlaces("%0.3f");
  std::cout << "\nSaving best and worst..\n" << std::endl;
  // Save however many images from the start and end.
  for(size_t i = 0; i < imagePaths.size(); ++i)
  {
    size_t iend = imagePaths.size() - 1 - i;

    size_t bestImageId = scores[i].first;
    size_t worstImageId = scores[iend].first;

    save_images(saveResultsPath, imagePaths[bestImageId], detections[bestImageId], "rank-" + boost::lexical_cast<std::string>(i) + "-score-" + (threeDecimalPlaces % scores[i].second).str());
    save_images(saveResultsPath, imagePaths[worstImageId], detections[worstImageId], "rank-" + boost::lexical_cast<std::string>(iend) + "-score-" + (threeDecimalPlaces % scores[iend].second).str());

    if(i > 50) break;
  }
}

void Evaluator::save_images(const std::string& saveResultsPath, const std::string& imagePath, const Detections& detections, const std::string& tag) const
{
  std::string saveResultsDir = saveResultsPath + "/images";
  if(!boost::filesystem::exists(saveResultsDir)) boost::filesystem::create_directories(saveResultsDir);

  const float detectionThreshold = 0.2f;

  cv::Mat3b im = cv::imread(imagePath, CV_LOAD_IMAGE_COLOR);
  std::string imageName = (boost::filesystem::path(imagePath)).stem().string();

// Ground truth
  Detections groundTruth = m_dataset->get_detections_from_image_name(imageName);

  // First save detections overlayed on a blank canvas.
  cv::Mat3b blankCanvas = cv::Mat::zeros(im.size(), CV_8UC3);
  cv::Mat3b gtOnBlankCanvas = DetectionUtil::overlay_detections(blankCanvas, groundTruth, 1, m_dataset->get_category_names(), m_dataset->get_palette());
  {
    std::string fileName = imageName + '-' + "groundtruth" + '-' + "onblack" + '-' + tag + ".png";
    std::string filePath = saveResultsDir + '/' + fileName;
    cv::imwrite(filePath, gtOnBlankCanvas, boost::assign::list_of(0));
  }

  // Next save detections overlayed on the original image.
  cv::Mat3b gtOnSource = DetectionUtil::overlay_detections(im, groundTruth, 1, m_dataset->get_category_names(), m_dataset->get_palette());
  {
    std::string fileName = imageName + '-' + "groundtruth" + '-' + "onsource" + '-' + tag + ".png";
    std::string filePath = saveResultsDir + '/' + fileName;
    cv::imwrite(filePath, gtOnSource, boost::assign::list_of(0));
  }

// Predictions

  // First save detections overlayed on a blank canvas.
  cv::Mat3b predOnBlankCanvas = DetectionUtil::overlay_detections(blankCanvas, detections, detectionThreshold, m_dataset->get_category_names(), m_dataset->get_palette());
  {
    std::string fileName = imageName + '-' + "predictions" + '-' + "onblack" + '-' + tag + ".png";
    std::string filePath = saveResultsDir + '/' + fileName;
    cv::imwrite(filePath, predOnBlankCanvas, boost::assign::list_of(0));
  }


  // First save detections overlayed on the original image.
  cv::Mat3b predOnSource = DetectionUtil::overlay_detections(im, detections, detectionThreshold, m_dataset->get_category_names(), m_dataset->get_palette());
  {
    std::string fileName = imageName + '-' + "predictions" + '-' + "onsource" + '-' + tag + ".png";
    std::string filePath = saveResultsDir + '/' + fileName;
    cv::imwrite(filePath, predOnSource, boost::assign::list_of(0));
  }
}

#if 0
double Evaluator::calculate_map_matlab(network& net, const std::string& saveResultsPath, VOCYear vocYear, VOCSplit vocSplit, const std::string& uniqueStamp) const
{
  write_settings_report(saveResultsPath, uniqueStamp, vocYear, vocSplit);

  const bool seenNonVOCData(false);
  const std::string competitionCode = m_dataset->get_competition_code(vocSplit, seenNonVOCData);
  std::vector<std::string> imagePaths = m_dataset->get_image_paths(vocYear, vocSplit, VOC_JPEG);

  std::vector<std::string> saveResultsPerCategoryFiles = get_save_results_per_category_files(saveResultsPath, competitionCode);

  // Save the detection results to text files.
  if(!boost::filesystem::exists(saveResultsPerCategoryFiles[0]))
  {
    TIME(
    std::vector<Detections> detections = DetectionUtil::detect_fast(net, imagePaths, m_ds);
    , seconds, fastDetection); std::cout << fastDetection << std::endl;

    if(detections.size() != imagePaths.size()) throw std::runtime_error("The number of detections and image paths shold be equal");

    std::vector<std::vector<NamedCategoryDetection> > namedCategoryDetections = convert_to_named_category_detections(imagePaths, detections);
    print_detections(namedCategoryDetections, saveResultsPerCategoryFiles, m_ds.categoryCount);

    // Write the results to file.
    //DetectionUtil::print_detections(imagePaths, detections, saveResultsPerCategoryFiles, m_ds.categoryCount);
   }

  return calculate_map_matlab(saveResultsPath, competitionCode, m_dataset->get_split_name(vocSplit));
}
#endif

//#################### PRIVATE MEMBER FUNCTIONS ####################

std::vector<double> Evaluator::calculate_ap_for_categories(const NameToVOCObjectsHash& nameToVOCObjectsHash, const std::vector<NamedCategoryDetections>& namedCategoryDetections, double overlapThreshold) const
{
  int categoryCount = static_cast<int>(namedCategoryDetections.size());
  std::vector<double> ap(categoryCount);
#pragma omp parallel for
  for(int c = 0; c < categoryCount; ++c)
  {
    ap[c] = calculate_ap_for_category(nameToVOCObjectsHash, namedCategoryDetections[c], c, overlapThreshold);
  }

  return ap;
}

double Evaluator::calculate_ap_for_category(const NameToVOCObjectsHash& nameToVOCObjectsHash, const NamedCategoryDetections& namedCategoryDetections, size_t categoryId, double overlapThreshold) const
{
  // Sort detections by decreasing confidence.
  std::vector<NamedCategoryDetection> dets = namedCategoryDetections;
  TupleComparator<2, NamedCategoryDetection> comp((std::greater<float>()));
  std::sort(dets.begin(), dets.end(), comp);

  std::set<std::string> alreadyDetected;
  size_t detCount(namedCategoryDetections.size());
  std::set<std::string> uniqueImageNames;
  std::vector<int> tp(detCount,0);
  std::vector<int> fp(detCount,0);
  for(size_t i = 0; i < detCount; ++i)
  {
    const NamedCategoryDetection& det = dets[i];
    const std::string& imageName = det.get<0>();
    const Shape& predShape = det.get<1>();
    //float score = det.get<2>();

    // Get the ground truth annotation for the image in which the detection was found.
    uniqueImageNames.insert(imageName);

    NameToVOCObjectsHash::const_iterator it = nameToVOCObjectsHash.find(imageName);
    if(it == nameToVOCObjectsHash.end()) throw std::runtime_error("Cound not find the objects for the specified image name");
    const std::vector<VOCObject>& objects = it->second;

    float maxOverlap(-std::numeric_limits<float>::max());
    size_t maxIndex(0);

    // Assign detection to ground truth object if any.
    for(size_t o = 0, gtObjectCount = objects.size(); o < gtObjectCount; ++o)
    {
      if(objects[o].categoryId != static_cast<int>(categoryId)) continue;

      const Shape& gtShape = objects[o].rep;

      float intersectionArea = predShape.calculate_intersection_area(gtShape);

      if(intersectionArea > 0)
      {
        float unionArea = predShape.area()
                          + gtShape.area()
                          - intersectionArea;

        float overlap = intersectionArea / unionArea;
        if(overlap > maxOverlap)
        {
          maxOverlap = overlap;
          maxIndex = o;
        }
      }
    }

    // Assign detection as true positive/don't care/false positive
    if(maxOverlap >= overlapThreshold)
    {
      const std::string key = imageName + boost::lexical_cast<std::string>(maxIndex);
      const VOCObject& gtObject = objects[maxIndex];
      if(!gtObject.difficult)
      {
        if(alreadyDetected.find(key) == alreadyDetected.end())
        {
          tp[i] = 1; // true positive
          alreadyDetected.insert(key);
        }
        else
        {
          fp[i] = 1; // false positive (multiple detection)
        }
      }
      else
      {} // don't care.
    }
    else
    {
      fp[i] = 1;
    }
  }

  size_t positiveCount(0);
  for(std::set<std::string>::iterator it = uniqueImageNames.begin(), iend = uniqueImageNames.end(); it != iend; ++it)
  {
    NameToVOCObjectsHash::const_iterator oit = nameToVOCObjectsHash.find(*it);
    if(oit == nameToVOCObjectsHash.end()) throw std::runtime_error("Cound not find the objects for the specified image name");
    positiveCount += VOCAnnotation::calculate_object_count(oit->second, categoryId);
  }

  // Compute the precision / recall
  std::vector<int> cumsumtp(tp.size(), 0);
  std::vector<int> cumsumfp(fp.size(), 0);
  std::partial_sum(tp.begin(), tp.end(), cumsumtp.begin(), std::plus<int>());
  std::partial_sum(fp.begin(), fp.end(), cumsumfp.begin(), std::plus<int>());

  std::vector<double> recall(tp.size() + 2, 0);
  recall.back() = 1;
  std::vector<double> precision(tp.size() + 2, 0);
  for(size_t i = 0; i < tp.size(); ++i)
  {
    recall[i+1] = cumsumtp[i] / static_cast<double>(positiveCount);
    precision[i+1] = cumsumtp[i] / static_cast<double>(cumsumfp[i] + cumsumtp[i]);
  }

  // Calculate the AP.
  double ap(0.0);
  for(size_t i = 1; i < recall.size(); ++i)
  {
    size_t iend = recall.size() - 1 - i;
    precision[iend] = std::max(precision[iend], precision[iend+1]);
    ap += (recall[iend+1] - recall[iend]) * precision[iend + 1];
  }

  return ap;
}

double Evaluator::calculate_map(const std::vector<NamedCategoryDetections>& perClassNamedCategoryDetections, const NameToVOCObjectsHash& nameToVOCObjectsHash, const std::string& saveResultsPath, const std::string uniqueStamp, double overlapThreshold) const
{
  std::cout << "\nPerforming the evaluation..\n" << std::endl;
  std::vector<double> ap = calculate_ap_for_categories(nameToVOCObjectsHash, perClassNamedCategoryDetections, overlapThreshold);

  // Save the results to a file.
  save_results_to_file(saveResultsPath, uniqueStamp, m_dataset->get_category_names(), ap, overlapThreshold);

  return Util::average_vector(ap);
}

Evaluator::NameToVOCObjectsHash Evaluator::create_name_to_objects_hash(const std::vector<std::string>& imagePaths) const
{
  std::vector<std::vector<VOCObject> > objectsPerImage(imagePaths.size());
  int pathCount = static_cast<int>(imagePaths.size());
#pragma omp parallel for
  for(int i = 0; i < pathCount; ++i)
  {
    if((i>0) && (i % 100 == 0)) std::cout << '.' << std::flush;
    boost::filesystem::path bpath(imagePaths[i]);
    std::string imageName = bpath.stem().string();
    objectsPerImage[i] = m_dataset->get_annotation_from_image_name(imageName)->get_objects();
  }
  std::cout << '\n';

  // Create a hash map from image names to annotation.
  NameToVOCObjectsHash nameToVOCObjectsHash;
  for(int i = 0; i < pathCount; ++i)
  {
    boost::filesystem::path bpath(imagePaths[i]);
    std::string imageName = bpath.stem().string();
    nameToVOCObjectsHash.insert(std::make_pair(imageName, objectsPerImage[i]));
  }

  return nameToVOCObjectsHash;
}

std::vector<NamedCategoryDetections> Evaluator::calculate_detections_per_category(network& net, const std::vector<std::string>& imagePaths) const
{
  // Calculate the detections and convert them to an appropriate format for evaluation.
  TIME(
  std::vector<Detections> detections = DetectionUtil::detect_fast(net, imagePaths, m_ds, m_shapeDescriptorCalculator);
  , seconds, detectionCalculationTime); std::cout << detectionCalculationTime;

  return convert_to_named_category_detections(imagePaths, detections);
}

void Evaluator::save_results_to_file(const std::string& saveResultsPath, const std::string& uniqueStamp, const std::vector<std::string>& categoryNames, const std::vector<double>& ap, double overlapThreshold) const
{
  boost::format oneDecimalPlace("%0.1f");
  std::string measureName = "ap@"+(oneDecimalPlace % overlapThreshold).str();
  PerformanceTable table(list_of(measureName));

  for(size_t i = 0; i < categoryNames.size(); ++i)
  {
    ParamSet params = map_list_of("Category",categoryNames[i]);
    std::map<std::string,PerformanceMeasure> result = map_list_of(measureName, PerformanceMeasure(ap[i]));
    table.record_performance(params, result);
  }

  std::string tableFile(saveResultsPath + "/results-table-" + measureName + ".txt");
  std::ofstream ofs(tableFile);
  table.output(ofs, " ");
  ofs.close();

  std::string resultsFile(saveResultsPath + "/results-m" + measureName + ".txt");
  boost::format tenDecimalPlaces("%0.10f");
  if(!boost::filesystem::exists(resultsFile))
  {
    std::ofstream ofs(resultsFile);
    std::string map = (tenDecimalPlaces % Util::average_vector(ap)).str();
    ofs << map;
    std::cout << "\nmap= " << map << '\n';
  }
}

#if 0
double Evaluator::calculate_map_matlab(const std::string& resultsPath, const std::string& competitionCode, const std::string& splitName) const
{
  // Use MATLAB to evaluate the results.
  const std::string resultsFilename("matlab-results.txt");
  const std::string saveResultsFile(resultsPath + '/' + resultsFilename);
  if(!boost::filesystem::exists(saveResultsFile))
  {
    // Get the mAP using the VOC matlab scripts.
    const std::string scriptFile = Util::resources_dir().string() + "/scripts/evaluate_detector.sh";
    if(!boost::filesystem::exists(scriptFile)) throw std::runtime_error("The file '" + scriptFile + "' could not be found");

    const std::string generateResultsCommand = scriptFile + ' ' + resultsPath + ' ' + resultsFilename + ' ' + competitionCode + ' ' + splitName;

    if(system(generateResultsCommand.c_str()) < 0) throw std::runtime_error("The evaluation script failed to execute");
  }

  // Read in the results generated by MATLAB.
  std::map<std::string, size_t> categoryId = m_dataset->get_category_name_to_id();
  std::ifstream ifs(saveResultsFile);
  if(!ifs) throw std::runtime_error("The file '" + saveResultsFile + "' could not be opened");
  std::vector<std::vector<std::string> > words = LineUtil::extract_word_lines(ifs, " ");
  const size_t categoryCount = m_dataset->get_category_names().size();
  std::vector<double> ap(categoryCount);

  if(words.size() != categoryCount) throw std::runtime_error("Number of lines in results file does not match the number of categories in the dataset");
  double apSum(0.0);
  for(size_t i = 0, size = words.size(); i < size; ++i)
  {
    std::cout << words[i][0] << ' ' << words[i][1] << '\n';
    // TODO: use performance measures, get the utils merged.
    //ap[MapUtil::lookup(categoryId, words[i][0])] = boost::lexical_cast<float>(words[i][1]);
    apSum += boost::lexical_cast<double>(words[i][1]);
  }

  return apSum/categoryCount;
}
#endif

std::vector<std::vector<NamedCategoryDetection> > Evaluator::convert_to_named_category_detections(const std::vector<std::string>& imagePaths, const std::vector<Detections>& detections, double minDetectionScoreThreshold) const
{
  if(detections.size() != imagePaths.size()) throw std::runtime_error("Number of image paths must equal to the number of detections");

  size_t categoryCount = m_dataset->get_category_names().size();
  std::vector<std::vector<NamedCategoryDetection> > namedCategoryDetections(categoryCount);
  for(size_t imageId = 0, imageCount = imagePaths.size(); imageId < imageCount; ++imageId)
  {
    boost::filesystem::path path(imagePaths[imageId]);
    std::string imageName = path.stem().string();

    for(size_t i = 0, detectionCount = detections[imageId].size(); i < detectionCount; ++i)
    {
      Shape shape = detections[imageId][i].first;
      std::vector<float> scores = detections[imageId][i].second;

      for(size_t c = 0; c < categoryCount; ++c)
      {
        if(scores[c] > minDetectionScoreThreshold)
        {
          namedCategoryDetections[c].push_back(boost::make_tuple(imageName, shape, scores[c]));
        }
      }
    }
  }

  return namedCategoryDetections;
}

double Evaluator::get_score_per_image(const NameToVOCObjectsHash& nameToVOCObjectsHash, const std::string& imagePath, const Detections& detections) const
{
  const double detectionThreshold(0.2f);
  const double overlapThreshold(0.5f);

  std::string imageName = (boost::filesystem::path(imagePath)).stem().string();
  NameToVOCObjectsHash::const_iterator it = nameToVOCObjectsHash.find(imageName);
  if(it == nameToVOCObjectsHash.end()) throw std::runtime_error("Could not find the objects for the specified image name");
  const std::vector<VOCObject>& objects = it->second;

  double tp(0.0);
  double fp(0.0);

  std::set<size_t> alreadyDetected;
  for(size_t i = 0; i < detections.size(); ++i)
  {
    const std::vector<float>& scores = detections[i].second;
    size_t categoryId = ArgUtil::argmax(scores);
    float score = scores[categoryId];
    if(score < detectionThreshold) continue;

    float maxOverlap(-std::numeric_limits<float>::max());
    size_t maxIndex(0);

    // Assign to a ground truth object if any.
    for(size_t o = 0; o < objects.size(); ++o)
    {
      if(objects[o].categoryId != static_cast<int>(categoryId)) continue;

      const Shape& gtShape = objects[o].rep;
      const Shape& predShape = detections[i].first;

      float intersectionArea = predShape.calculate_intersection_area(gtShape);

      if(intersectionArea > 0)
      {
        float unionArea = predShape.area()
                          + gtShape.area()
                          - intersectionArea;

        float overlap = intersectionArea / unionArea;
        if(overlap > maxOverlap)
        {
          maxOverlap = overlap;
          maxIndex = o;
        }
      }
    }

    // Assign detection as true positive/don't care/false positive
    if(maxOverlap >= overlapThreshold)
    {
      const VOCObject& gtObject = objects[maxIndex];
      if(!gtObject.difficult)
      {
        if(alreadyDetected.find(maxIndex) == alreadyDetected.end())
        {
          tp += (1.0 * maxOverlap); // true positive
          alreadyDetected.insert(maxIndex);
        }
        else
        {
          fp += 1.0; // false positive (multiple detection)
        }
      }
      else
      {} // don't care.
    }
    else
    {
      fp += 1.0;
    }
  }

  double precision = tp / (tp + fp);
  double recall = tp / static_cast<double>(objects.size());

  double f1 = (2 * precision * recall) / (precision + recall + std::numeric_limits<double>::min());

#if 1
  boost::format sdp("%0.6f");

  std::string line;
  line += imageName + ':';
  line += " tp:" + (sdp % tp).str() + ',';
  line += " fp:" + (sdp % fp).str() + ',';
  line += " objs:" + boost::lexical_cast<std::string>(objects.size()) + ',';
  line += " dets:" + boost::lexical_cast<std::string>(detections.size()) + ',';
  line += " prec:" + (sdp % precision).str() + ',';
  line += " rec:" + (sdp % recall).str() + ',';
  line += " f1:" + (sdp % f1).str() + ',';
  line += '\n';

  std::ofstream debugFile("/tmp/scoresPerImage.txt", std::fstream::app);
  debugFile << line;
  std::cout << line;
  /*
  std::cout << imageName << std::endl;
  std::cout << "tp:" << tp << " fp:" << fp << " objs:" << objects.size() << " dets:" << detections.size() << std::endl;
  std::cout << "p:" << precision << " r:" << recall << " f1:" << f1 << std::endl;
  std::cout << std::endl;
  */
#endif


  if(std::isnan(f1) | std::isinf(f1)) f1 = 0.0;

  f1 += (tp * 0.1); // favour reuslts which have more true positives.
  return f1;
}

std::vector<std::string> Evaluator::get_save_results_per_category_files(const std::string& saveResultsPath, const std::string& competitionCode) const
{
  const std::vector<std::string> categoryNames = m_dataset->get_category_names();
  const size_t categoryCount = categoryNames.size();
  std::vector<std::string> saveResultsPerCategoryFiles;
  for(size_t i = 0; i < categoryCount; ++i)
  {
    saveResultsPerCategoryFiles.push_back(saveResultsPath + '/' + competitionCode + '_' + categoryNames[i] + ".txt");
  }

  return saveResultsPerCategoryFiles;
}

void Evaluator::print_detections(const std::vector<std::vector<NamedCategoryDetection> >& namedCategoryDetections, const std::vector<std::string>& saveResultsPerCategoryFiles, size_t categoryCount) const
{
  if(saveResultsPerCategoryFiles.size() != categoryCount) throw std::runtime_error("number of result paths must be the same as the nubmer of categories");

  std::vector<std::ofstream> ofStreams(categoryCount);
  for(size_t c = 0; c < categoryCount; ++c)
  {
    ofStreams[c].open(saveResultsPerCategoryFiles[c]);
    const std::vector<NamedCategoryDetection>& classDetections = namedCategoryDetections[c];

    for(size_t d = 0; d < classDetections.size(); ++d)
    {
      const NamedCategoryDetection& det = classDetections[d];
      std::string imageName = det.get<0>();
      VOCBox b = det.get<1>().get_voc_box();
      double score = det.get<2>();
      ofStreams[c] << imageName << ' ' << score << ' ' << b.xmin << ' ' << b.ymin << ' ' << b.xmax << ' ' << b.ymax << '\n';
    }
  }
}

void Evaluator::write_settings_report(const std::string& saveResultsPath, const std::string& uniqueStamp, VOCYear vocYear, VOCSplit vocSplit) const
{
  // First write a report with the settings used to generate these results.
  const std::string saveSettingsFile(saveResultsPath + "/settings.txt");
  if(!boost::filesystem::exists(saveSettingsFile))
  {
    std::ofstream settingsFile(saveSettingsFile);
    settingsFile << m_dataset << '\n';
    settingsFile << m_dataset->get_year_name(vocYear) << '\n';
    settingsFile << m_dataset->get_split_name(vocSplit) << '\n';
    settingsFile << m_ds << '\n';
    settingsFile << uniqueStamp << '\n';
  }
}
