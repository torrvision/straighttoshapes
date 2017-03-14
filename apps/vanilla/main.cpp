/**
 * vanilla: main.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "Demo.h"
#include "Evaluator.h"
#include "Tester.h"
#include "Trainer.h"
#include "Util.h"

#include "data/TrainingDataGenerator.h"

#include "capture/Capture.h"
#include "core/DetectionSettings.h"
#include "dataset/VOCDatasetDetection.h"
#include "dataset/VOCDatasetSegmentation.h"
#include "dataset/VOCDatasetSBD.h"
#include "dataset/VOCDatasetUtil.h"
#include "dataset/COCODatasetInstance.h"

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include <boost/format.hpp>

#include <boost/assign/list_of.hpp>
using namespace boost::assign;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <boost/property_tree/ptree.hpp>
using namespace boost::property_tree;

#include <boost/asio/ip/host_name.hpp>
using namespace boost::asio::ip;

#include <boost/shared_ptr.hpp>

#include <darknet/parser.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <tvgutil/timing/TimeUtil.h>
#include <tvgutil/timing/Timer.h>
#include <tvgutil/containers/LimitedContainer.h>
#include <tvgutil/numbers/NumberSequenceGenerator.h>
#include <tvgutil/persistence/PropertyUtil.h>
#include <tvgutil/persistence/LineUtil.h>
using namespace tvgutil;

#include <tvgshape/ShapeDescriptorCalculator.h>
#include <tvgshape/BinaryMaskShapeDescriptorCalculator.h>
#include <tvgshape/RadialShapeDescriptorCalculator.h>
#ifdef WITH_TORCH
#include <tvgshape/AEShapeDescriptorCalculator.h>
#endif
using namespace tvgshape;

#include <tvgplot/PaletteGenerator.h>
using namespace tvgplot;

// #################### TYPES ####################

struct CommandLineArguments
{
  std::string dataDir;
  std::string dataset;
  bool debugFlag;
  float detectionThreshold;
  std::string encoding;
  int gpuId;
  std::string imagePath;
  std::string mode;
  std::string networkConfigurationFile;
  std::string saveDir;
  unsigned int seed;
  size_t shapeparams;
  std::string task;
  std::string timeStamp;
  std::string videoFile;
  std::string weightsFile;
};

// #################### ENUMS ####################

enum Mode
{
  TRAIN,
  TEST,
  EVALUATE,
  DEMO
};

// #################### FUNCTIONS ####################

std::string get_unique_stamp(const CommandLineArguments& args)
{
  std::string stamp;
  stamp += "-networkConfigurationFile=" + args.networkConfigurationFile;
  stamp += "- -weightsFile=" + args.weightsFile + '-';

  return stamp;
}

std::ostream& operator<<(std::ostream& os, const CommandLineArguments& args)
{
  os << "dataDir: " << args.dataDir << '\n';
  os << "dataset: " << args.dataset << '\n';
  os << "debugFlag: " << args.debugFlag << '\n';
  os << "detectionThreshold: " << args.detectionThreshold << '\n';
  os << "encoding: " << args.encoding << '\n';
  os << "gpuId: " << args.gpuId << '\n';
  os << "imagePath: " << args.imagePath << '\n';
  os << "mode: " << args.mode << '\n';
  os << "networkConfgurationFile: " << args.networkConfigurationFile << '\n';
  os << "saveDir: " << args.saveDir << '\n';
  os << "seed: " << args.seed << '\n';
  os << "shapeparams: " << args.shapeparams << '\n';
  os << "task: " << args.task << '\n';
  os << "timeStamp: " << args.timeStamp << '\n';
  os << "videoFile: " << args.videoFile << '\n';
  os << "weightsFile: " << args.weightsFile << '\n';
  return os;
}

Mode get_mode(const std::string& mode)
{
  if(mode == "train") return TRAIN;
  else if(mode == "test") return TEST;
  else if(mode == "evaluate") return EVALUATE;
  else if(mode == "demo") return DEMO;
  else throw std::runtime_error("Invalid mode");
}

std::string get_weights_file(const std::string& dataDir, const std::string& weightsFile)
{
  if(!boost::filesystem::exists(weightsFile))
  {
    // Check if the path is relative to the data directory.
    std::string path = dataDir + "/models/" + weightsFile;
    if(!exists(path))
    {
      throw std::runtime_error("Could not find: " + weightsFile);
    }
    else
    {
      return path;
    }
  }
  else return weightsFile;
}

bool arguments_are_valid(CommandLineArguments& args)
{
  if(!boost::filesystem::exists(args.dataDir))
  {
    std::cerr << "dataDir: '" << args.dataDir << "' - does not exist\n\n";
    return false;
  }

  if(args.dataset == "coco" && args.networkConfigurationFile == "yolo.cfg")
  {
    std::cerr << "Warning using yolo-coco.cfg as default for the coco dataset\n";
    args.networkConfigurationFile = "yolo-coco.cfg";
  }

  if(!exists(args.networkConfigurationFile))
  {
    // Check if the path is relative to the resources directory.
    std::string path = Util::resources_dir().string() + "/cfg/" + args.networkConfigurationFile;
    if(!exists(path))
    {
      std::cerr << "could not find: " << args.networkConfigurationFile << '\n';
      return false;
    }
    else
    {
      args.networkConfigurationFile = path;
    }
  }
  //
  // Set up the parameters for the specific task.
  if(args.task == "detection")
  {
    if(args.encoding != "bbox")
      std::cout << "Warning, changing the embedding to 'bbox' from " << args.encoding << std::endl;

    args.encoding == "bbox";
    if(args.weightsFile.empty()) args.weightsFile = "sbd-yolo-bbox-c20-sp0-train.weights";
  }
  else if(args.task == "shapeprediction")
  {
    if((args.encoding == "bbox") || args.encoding == "mask")
    {
      std::cout << "Warning, changing the embedding to 'mask' form " << args.encoding << std::endl;
      args.encoding = "mask";
      args.shapeparams = 256;
      if(args.weightsFile.empty()) args.weightsFile = "sbd-yolo-mask-c20-sp256-trainval-demo.weights";
    }
    else if(args.encoding == "embedding")
    {
      if(args.shapeparams == 256)
      {
        throw std::runtime_error("Specify an embedding dimension of 20 or 50, '--sp 20'");
      }
      else if(args.shapeparams == 20 && args.weightsFile.empty())
      {
        args.weightsFile = "sbd-yolo-embedding-c20-sp20-train.weights";
      }
      else if(args.shapeparams == 50 && args.weightsFile.empty())
      {
        args.weightsFile = "sbd-yolo-embedding-c20-sp50-trainval-demo.weights";
      }
    }
    else throw std::runtime_error("Invalid encoding: " + args.encoding);
  }
  else throw std::runtime_error("Invalid task: " + args.task);

  args.weightsFile = get_weights_file(args.dataDir, args.weightsFile);

  // This will throw if mode is not recognised.
  get_mode(args.mode);

  if(!args.imagePath.empty())
  {
    if(!exists(args.imagePath)) throw std::runtime_error("Cound not find: " + args.imagePath);
  }

  return true;
}

bool parse_command_line(int argc, char *argv[], CommandLineArguments& args)
{
  // Specigy the possible options.
  po::options_description genericOptions("Generic options");
  genericOptions.add_options()
    ("help", "produce help message")
    ("dataDir,d", po::value<std::string>(&args.dataDir), "data directory")
    ("dataset", po::value<std::string>(&args.dataset)->default_value(""), "dataset name: [vocdet, vocseg, sbd, coco]")
    ("debug", po::bool_switch(&args.debugFlag)->default_value(false), "debug flag")
    ("detectionTreshold,t", po::value<float>(&args.detectionThreshold)->default_value(0.001f), "detection threshold")
    ("encoding", po::value<std::string>(&args.encoding)->default_value("bbox"), "shape encoding: [bbox, mask, maskdt, radial, embedding]")
    ("gpuId,g", po::value<int>(&args.gpuId)->default_value(0), "gpu id")
    ("image,i", po::value<std::string>(&args.imagePath)->default_value(""), "image path")
    ("mode,m", po::value<std::string>(&args.mode), "program mode: [train, test, evaluate, demo]")
    ("networkConfigurationFile,n", po::value<std::string>(&args.networkConfigurationFile)->default_value("yolo.cfg"), "network configuration file")
    ("saveDir", po::value<std::string>(&args.saveDir)->default_value(""), "directory to save demo output")
    ("seed", po::value<unsigned int>(&args.seed)->default_value(12345), "seed for random number generation")
    ("shapeparams", po::value<size_t>(&args.shapeparams)->default_value(256), "The number of parameters in the shape encoding")
    ("task", po::value<std::string>(&args.task)->default_value("detection"), "task [detection, shapeprediction)")
    ("timeStamp", po::value<std::string>(&args.timeStamp)->default_value(""), "time stamp")
    ("videoFile", po::value<std::string>(&args.videoFile)->default_value(""), "path to a video file")
    ("weightsFile,w", po::value<std::string>(&args.weightsFile)->default_value(""), "initial weights file")
    ;

  // Actually parse the command line.
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, genericOptions), vm);
  po::notify(vm);

  // If the user specifies the --help flag, print a help message.
  if(vm.count("help"))
  {
    std::cout << genericOptions << '\n';
    return false;
  }

  if(!arguments_are_valid(args))
  {
    std::cout << genericOptions << '\n';
    return false;
  }

  return true;
}

// #################### MAIN ####################
#if 0

cv::Mat stitch(const std::vector<cv::Mat>& images)
{
  const cv::Mat& first = images[0];
  size_t imageCount = images.size();
  size_t rows = first.rows;
  size_t cols = first.cols;

  // Create new canvas.
  cv::Mat result = cv::Mat::zeros(rows, cols * imageCount, first.type());

  for(size_t i = 0; i < imageCount; ++i)
  {
    cv::Rect roi(i*cols, 0, cols, rows);
    cv::Mat roiResult = result(roi);
    cv::add(roiResult, images[i], roiResult);
  }

  return result;
}

bool is_hidden_file(const boost::filesystem::path& p)
{
  boost::filesystem::path::string_type name = p.filename().native();
  if(name != ".." && name != "." && name[0] == '.') return true;

  return false;
}

std::vector<std::string> directory_list(const std::string& dir)
{
  std::vector<std::string> result;

  boost::filesystem::path p(dir);
  if(exists(p))
  {
    if(is_regular_file(p)) throw std::runtime_error("regular file");

    if(is_directory(p))
    {
      std::vector<boost::filesystem::path> paths;
      std::copy(directory_iterator(p), directory_iterator(), std::back_inserter(paths));
      std::sort(paths.begin(), paths.end());

      for(std::vector<boost::filesystem::path>::const_iterator it = paths.begin(); it != paths.end(); ++it)
      {
        boost::filesystem::path filename = it->filename();
        if(!is_hidden_file(filename))
        {
          result.push_back(it->string());
        }
      }
    }
    else throw std::runtime_error("is not a directory or regular file");
  }
  else throw std::runtime_error("does not exist");

  return result;
}

int main()
{
  // The names of the directories with images to compare.
  std::string videoName("jackTheSheepWhoThinksHesADog");
  std::string frames("428-508");
  std::vector<std::string> directories = list_of
      //("/media/mikesapi/DATASETDISK/ms-workspace/youtubeVideosResults/clips/"+videoName+"/"+frames+"/embedding20")
      ("/media/mikesapi/DATASETDISK/ms-workspace/youtubeVideosResults/clips/"+videoName+"/"+frames+"/embedding50")
      ("/media/mikesapi/DATASETDISK/ms-workspace/youtubeVideosResults/clips/"+videoName+"/"+frames+"/mask256")
      ;
  const size_t directoryCount = directories.size();

  std::vector<std::vector<std::string> > pathsArray;
  std::vector<std::string> windowNames;
  // Get the list of images in each folder.
  for(size_t i = 0; i < directories.size(); ++i)
  {
    std::vector<std::string> paths = directory_list(directories[i]);
    pathsArray.push_back(paths);
    std::string directoryName = (boost::filesystem::path(directories[i])).stem().string();
    windowNames.push_back(directoryName + "stream" + boost::lexical_cast<std::string>(i));
    cv::namedWindow(windowNames[i], cv::WINDOW_NORMAL);
    cv::moveWindow(windowNames[i], i*700,0);
  }

  int maxIndex = pathsArray[0].size();
  int i(0);
  bool valid(true);
  //while(valid)
  for(size_t i = 0; i < pathsArray[0].size(); ++i)
  {
    std::vector<cv::Mat> images;
    for(size_t j = 0; j < directoryCount; ++j)
    {
      images.push_back(cv::imread(pathsArray[j][i], CV_LOAD_IMAGE_COLOR));
    }

    for(size_t j = 0; j < directoryCount; ++j)
    {
      std::string imageName = (boost::filesystem::path(pathsArray[j][i])).stem().string();
      cv::imshow(windowNames[j], images[j]);
    }

#if 1
    cv::Mat results = stitch(images);

    std::string outputDir = "/tmp/result/";
    if(!boost::filesystem::exists(outputDir)) boost::filesystem::create_directories(outputDir);
    std::string fileName = (boost::filesystem::path(pathsArray[0][i])).stem().string() + ".png";
    std::string filePath = outputDir + '/' + fileName;
    cv::imwrite(filePath, results, boost::assign::list_of(0));
#else
    char key = cv::waitKey();

    if(key == 'l')
    {
      ++i;
    }
    else if(key == 'h')
    {
      --i;//%i = i - 2;
    }
    else if(key == 'q')
    {
      break;
    }

    if(i < 0) i = maxIndex - 1;
    if(i > (maxIndex - 1)) i = 0;//.. maxIndex - 1;
#endif
  }

  cv::destroyAllWindows();
  return 0;
}
#endif
#if 0
#include "core/MovingVectorAverage.h"

int main()
{
  size_t vectorSize(20000);
  MovingVectorAverage<float> mva(3, std::vector<float>(vectorSize,0));

  std::vector<float> average;
  TIME(
  for(size_t i = 0; i < 1000; ++i)
  {
    average = mva.push(std::vector<float>(vectorSize,static_cast<float>(i % 5)));
  }
  , milliseconds, movingAverageTime);
  std::cout << "average: " << make_limited_container(average, 10) << std::endl;
  std::cout << movingAverageTime << std::endl;

  return 0;
}
#endif

#if 0
int main()
{
  boost::shared_ptr<Dataset> dataset(new VOCDatasetSBD("/home/mikesapi/desktop/yolo/datasets/voc"));
  VOCSplit vocSplit(VOC_TRAIN);
  std::string splitName(dataset->get_split_name(vocSplit));
  std::string saveDir = "/tmp/semanticBoundariesDataset/" + splitName;

  if(!boost::filesystem::exists(saveDir)) boost::filesystem::create_directories(saveDir);
  const std::vector<std::string>& imagePaths = dataset->get_image_paths(VOC_2012, vocSplit, VOC_JPEG);
  std::cout << imagePaths.size() << std::endl;

  for(size_t i = 0; i < imagePaths.size(); ++i)
  {
    //std::cout << imagePaths[i] << '\n';
    VOCAnnotation_CPtr annotation = dataset->get_annotation_from_image_path(imagePaths[i]);
    /*
    std::cout << annotation->imageName << std::endl;
    dataset->visualise_annotation(imagePaths[i]);
    cv::waitKey();
    cv::waitKey(5); cv::destroyAllWindows(); cv::waitKey(5);
    */
    annotation->save(saveDir);
  }
  return 0;
}
#endif

#if 0
int main()
{
  boost::shared_ptr<Dataset> dataset(new VOCDatasetSegmentation("/home/mikesapi/desktop/yolo/datasets/voc"));

  //boost::shared_ptr<Dataset> dataset(new COCODatasetInstance("/home/mikesapi/desktop/yolo/datasets/coco"));
  const std::vector<std::string>& trainImagePaths = dataset->get_image_paths(COCO_2014, VOC_TRAIN, VOC_JPEG, 5000);
  const std::vector<std::string>& valImagePaths = dataset->get_image_paths(COCO_2014, VOC_VAL, VOC_JPEG, 5000);

  std::cout << "Number of training images: " << trainImagePaths.size() << std::endl;
  std::cout << "Number of validation images: " << valImagePaths.size() << std::endl;

  return 0;
}
#endif

#if 0
int main()
{
  boost::shared_ptr<VOCDataset> dataset(new VOCDatasetSegmentation("/home/mikesapi/desktop/yolo/datasets/voc"));
  std::cout << make_limited_container(dataset->get_palette(), 30) << '\n' << std::endl;
  std::cout << make_limited_container(dataset->get_colour_to_category_id(), 30) << '\n' << std::endl;
  std::cout << make_limited_container(dataset->get_category_id_to_colour(), 30) << '\n' << std::endl;
  return 0;
}
#endif
#if 0
int main()
{
  //boost::shared_ptr<Dataset> dataset(new VOCDatasetDetection("/media/mikesapi/DATASETDISK/ms-workspace/detection/yolo/datasets/voc"));
  //boost::shared_ptr<Dataset> dataset(new VOCDatasetSegmentation("/media/mikesapi/DATASETDISK/ms-workspace/detection/yolo/datasets/voc"));
  boost::shared_ptr<Dataset> dataset(new VOCDatasetSBD("/media/mikesapi/DATASETDISK/ms-workspace/detection/yolo/datasets/voc"));
  std::vector<std::string> trainPaths = dataset->get_image_paths(VOC_2012, VOC_TRAIN, VOC_JPEG);

  //boost::shared_ptr<Dataset> dataset(new COCODatasetInstance("/media/mikesapi/DATASETDISK/ms-workspace/detection/yolo/datasets/coco"));
  //std::vector<std::string> trainPaths = dataset->get_image_paths(COCO_2014, VOC_TRAIN, VOC_JPEG);

  size_t paramsPerShapeEncoding(256);
  boost::optional<ShapeDescriptorCalculator_CPtr> shapeDescriptorCalculator(nullptr);

#if 0
  //shapeDescriptorCalculator = ShapeDescriptorCalculator_CPtr(new RadialShapeDescriptorCalculator());
  shapeDescriptorCalculator = ShapeDescriptorCalculator_CPtr(new BinaryMaskShapeDescriptorCalculator());
#else
  paramsPerShapeEncoding = 50;
  static lua_State *L;
  static bool done = false;
  if(!done)
  {
    L = luaL_newstate();
    done = true;
  }
  const size_t descriptorSize = 50;

  shapeDescriptorCalculator = ShapeDescriptorCalculator_CPtr(new AEShapeDescriptorCalculator(L, Util::resources_dir().string() + "/torch/autoenc.lua", "/media/mikesapi/DATASETDISK/ms-workspace/detection/yolo/models/autoencoder", descriptorSize));
#endif

  TrainingDataGenerator dataGenerator(trainPaths, dataset, 448, 448, DataTransformationFactory::Settings(), 1234, shapeDescriptorCalculator);

  const bool onlyObjectness(false);
  const bool useNms(true);
  const float overlapThreshold(0.5f);
  float shapeScale(0.1f);

  DetectionSettings detectionSettings(
    dataset->get_category_names().size(), // categoryCount
    2,    // boxesPerCell
    0.01, // detectionThreshold
    "mask", // encoding
    7,    // gridSideLength (gridSideLength)
    useNms, // nms
    4,    // paramsPerBox
    1,    // paramsPerConfidenceScore
    paramsPerShapeEncoding,    // paramsPerShapeEncoding
    onlyObjectness, //onlyObjectness
    overlapThreshold, // overlapThreshold
    shapeScale,          // The factor by which to scale the shape error derivatives
    true // useSquare
    );

  const size_t imagesPerDatum(16);
  for(size_t i = 0; i < 100; ++i)
  {
    std::cout << i << '\n';
    TIME(
    Datum datum = dataGenerator.generate_training_datum(imagesPerDatum, 0.4, detectionSettings);
    , milliseconds, datumTime); std::cout << datumTime << '\n';
    //dataGenerator.debug_training_datum(datum, imagesPerDatum, detectionSettings);
  }

  return 0;
}
#endif

void modify_line(std::string& line, const std::string& paramName, const std::string& paramValue)
{
  // First check if you have the correct line.
  if(line.find(paramName) == std::string::npos) throw std::runtime_error("The parameter '" + paramName + "' is not on this line:\n" + line);

  line = paramName + '=' + paramValue;
}

#define blc(x) boost::lexical_cast<std::string>(x)
std::string create_configuration_file(const std::string& networkConfigurationFile, size_t batch, size_t subdivisions, const DetectionSettings& ds)
{
  // Read in the configuration file.
  std::ifstream ifs(networkConfigurationFile);
  std::vector<std::string> lines = LineUtil::extract_lines(ifs);

  modify_line(lines[1], "batch", blc(batch));

  modify_line(lines[2], "subdivisions", blc(subdivisions));

  size_t connections = ds.gridSideLength * ds.gridSideLength
                    * ((ds.paramsPerConfidenceScore + ds.paramsPerBox + ds.paramsPerShapeEncoding)*ds.boxesPerCell + ds.categoryCount);
  modify_line(lines[217], "output", blc(connections));

  const std::string sClasses = blc(ds.categoryCount);
  modify_line(lines[221], "classes", sClasses);

  const std::string sShapeParams = blc(ds.paramsPerShapeEncoding);
  modify_line(lines[223], "shapeparams", sShapeParams);

  const std::string sShapeScale = blc(ds.shapeScale);
  modify_line(lines[234], "shape_scale", sShapeScale);

  std::string dir = (boost::filesystem::path(networkConfigurationFile)).parent_path().string();
  std::string stem = (boost::filesystem::path(networkConfigurationFile)).stem().string();
  std::string ext = (boost::filesystem::path(networkConfigurationFile)).extension().string();

  std::string newFile = dir + '/' + stem
    + '-' +ds.encoding
    + '-' + 'c'+sClasses
    + '-' + "sp"+sShapeParams
    + ext;
  std::ofstream ofs(newFile);
  LineUtil::output_lines(ofs, lines);

  // Save the configuration file.
  return newFile;
}
#undef BLC

#if 1
int main(int argc, char *argv[])
{
  // Parse the command-line arguments
  CommandLineArguments args;
  if(!parse_command_line(argc, argv, args))
  {
    return 0;
  }
  std::cout << "Command-line arguments:\n" << args << std::endl;

  // Seeds.
  //const unsigned int seed = 12345; //Seed for the boost random number generator;
  srand(args.seed); // time(0); Initialise global seed for rand();

  // Initialise the network and load the initial weights.
#ifdef WITH_CUDA
  cudaSetDevice(args.gpuId);
#endif

  // Create the dataset.
  boost::shared_ptr<Dataset> dataset;
  VOCYear year;
  size_t epochCount;
  size_t maxImagesToEvaluateOn(10000);
  if(args.dataset == "vocdet")
  {
    dataset.reset(new VOCDatasetDetection(args.dataDir + "/datasets/voc"));
    if(args.task != "detection") throw std::runtime_error("Cannot run dataset " + args.dataset + " with task: " + args.task);
    if(args.encoding != "bbox") throw std::runtime_error("Cannot run dataset " + args.dataset + " with encoding: " + args.encoding);
    year = VOC_2012;
    epochCount = 250;
  }
  else if(args.dataset == "vocseg")
  {
    dataset.reset(new VOCDatasetSegmentation(args.dataDir + "/datasets/voc"));
    year = VOC_2012;
    epochCount = 800;
  }
  else if(args.dataset == "sbd")
  {
    dataset.reset(new VOCDatasetSBD(args.dataDir + "/datasets/voc"));
    year = VOC_2012;
    epochCount = 500;
  }
  else if(args.dataset == "coco")
  {
    dataset.reset(new COCODatasetInstance(args.dataDir + "/datasets/coco"));
    year = COCO_2014;
    epochCount = 150;
    maxImagesToEvaluateOn = 5000;
  }
  else if(args.dataset.empty() && (args.mode == "train" || args.mode == "evaluate")) throw std::runtime_error("Invalid dataset: " + args.dataset);

  // Set up the parameters for the specific embedding.
  float shapeScale(0.1f);
  boost::optional<ShapeDescriptorCalculator_CPtr> shapeDescriptorCalculator(nullptr);
  if(args.encoding == "bbox")
  {
    args.shapeparams = 0;
    shapeScale = 0;
  }
  else if(args.encoding == "mask" || args.encoding == "maskdt")
  {
    shapeScale = 0.1f;
    shapeDescriptorCalculator = ShapeDescriptorCalculator_CPtr(new BinaryMaskShapeDescriptorCalculator());
  }
  else if(args.encoding == "radial")
  {
    shapeScale = 1.0f;
    shapeDescriptorCalculator = ShapeDescriptorCalculator_CPtr(new RadialShapeDescriptorCalculator());
  }
#ifdef WITH_TORCH
  else if(args.encoding == "embedding")
  {
    shapeScale = 0.15f;
    static lua_State *L;
    static bool done = false;
    if(!done)
    {
      L = luaL_newstate();
      done = true;
    }

    shapeDescriptorCalculator = ShapeDescriptorCalculator_CPtr(new AEShapeDescriptorCalculator(L, Util::resources_dir().string() + "/torch/autoenc.lua", args.dataDir + "/models/autoencoder", args.shapeparams));
  }
#endif
  else throw std::runtime_error("Invalid embedding: " + args.encoding);

  // This needs to change if the embedding is smaller than the nubmer of parameters in the shape mask.
  const bool onlyObjectness(false);
  const bool useNms(true);
  const float overlapThreshold(0.5f);
  const size_t boxesPerCell(2);
  const size_t gridSideLength(7);
  const size_t paramsPerBox(4);
  const bool useSquare(true);

  size_t categoryCount(20); // default from pascal
  if(dataset) categoryCount = dataset->get_category_names().size();
  DetectionSettings detectionSettings(
    categoryCount,           // categoryCount
    boxesPerCell,            // boxesPerCell
    args.detectionThreshold, // detectionThreshold
    args.encoding,           // encoding
    gridSideLength,      // gridSideLength (gridSideLength)
    useNms,              // nms
    paramsPerBox,        // paramsPerBox
    1,                   // paramsPerConfidenceScore
    args.shapeparams,         // paramsPerShapeEncoding
    onlyObjectness,      //onlyObjectness
    overlapThreshold,    // overlapThreshold
    shapeScale,          // The factor by which to scale the shape error derivatives
    useSquare            // useSquare
    );

  std::cout << detectionSettings << std::endl;

  // Read and modify the yolo configuration file to match experiment parameters.
  size_t batch(1);
  size_t subdivisions(1);
  if(args.mode == "train")
  {
    batch = 64; subdivisions = 8;
    if(host_name() == "ms-tvg-workstation"){ batch = 64; subdivisions = 4; }
    if(host_name() == "mikesapi-tvg-laptop"){ batch = 64; subdivisions = 8; }
    if(host_name() == "sjvision"){ batch = 8; subdivisions = 2; }
  }

  std::string modifiedNetworkConfigFile = create_configuration_file(args.networkConfigurationFile, batch, subdivisions, detectionSettings);
  std::string configurationName = (boost::filesystem::path(modifiedNetworkConfigFile)).stem().string();

  // Create the network and load the weights.
  network net = parse_network_cfg(const_cast<char*>(modifiedNetworkConfigFile.c_str()));

  if((args.weightsFile.find(configurationName) == std::string::npos) && (args.weightsFile.find("extraction") == std::string::npos))
  {
    // TODO try to load a default.
    throw std::runtime_error("the weights file should have: " + configurationName + " in its filename");

  }
  load_weights(&net, const_cast<char*>(args.weightsFile.c_str()));

  // Get a time-stamp to uniquely identify this run of experiments.
  std::string timeStamp;
  if(args.timeStamp.empty()) timeStamp = TimeUtil::get_iso_timestamp();
  else timeStamp = args.timeStamp;
  std::string experimentUniqueStamp = args.mode + '-' + args.dataset + '-' + configurationName + '-' + timeStamp;

  Mode mode = get_mode(args.mode);// VISUALISE_DETECTIONS;
  if(mode != TRAIN)
  {
    // Set up the network.
    set_batch_network(&net, 1);
  }

  switch (mode)
  {
  case TRAIN:
    {
      const bool debugFlag = true;
      size_t maxDebugEvalImages(2000);
      Trainer trainer(dataset, year, detectionSettings, experimentUniqueStamp, debugFlag, args.seed, shapeDescriptorCalculator, maxDebugEvalImages);
      trainer.train(net, epochCount);
      break;
    }

  case TEST:
    {
      std::vector<std::string> imagePaths;
      if(exists(args.imagePath))
      {
        imagePaths = list_of(args.imagePath).to_container(imagePaths);
      }
      else if(dataset)
      {
        imagePaths = dataset->get_image_paths(VOC_2012, VOC_VAL, VOC_JPEG);
        //imagePaths = dataset->get_image_paths(VOC_2012, VOC_TRAIN, VOC_JPEG);
      }
      else throw std::runtime_error("include an image path or specify a dataset");

      Tester::visualise_detections(net, imagePaths, dataset, detectionSettings, shapeDescriptorCalculator);
      break;
    }

  case EVALUATE:
    {
      boost::format fourDecimalPlaces("%0.4f");
      std::string saveResultsPath = dataset->get_dir_in_results(experimentUniqueStamp);


      TIME(

#define MAPVOL
#if defined(MAP)
      Evaluator vocDetectionEvaluator(dataset, detectionSettings, shapeDescriptorCalculator);
      const double overlapThreshold(0.5);
      float map = vocDetectionEvaluator.calculate_map(net, saveResultsPath, year, VOC_VAL, get_unique_stamp(args), overlapThreshold, maxImagesToEvaluateOn);
      std::cout << "\nmAP: " << map << std::endl;

#elif defined(MAPVOL)
      Evaluator vocDetectionEvaluator(dataset, detectionSettings, shapeDescriptorCalculator);
      // Standard overlap threshold from ECCV2014 Hariharan evaluation on Pascal + SBD.
      std::vector<double> overlapThresholds = NumberSequenceGenerator::generate_stepped(0.1, 0.1, 0.9);
      if(args.dataset == "coco")
      {
        overlapThresholds = NumberSequenceGenerator::generate_stepped(0.5, 0.05, 0.95);
      }

      float mapVol = vocDetectionEvaluator.calculate_map_vol(net, saveResultsPath, year, VOC_VAL, get_unique_stamp(args), overlapThresholds, maxImagesToEvaluateOn);
      std::cout << "\nmAPVol: " << mapVol << std::endl;

#elif defined(SAVETOPBOTTOM)
      detectionSettings.detectionThreshold = 0.2;
      Evaluator vocDetectionEvaluator(dataset, detectionSettings, shapeDescriptorCalculator);
      vocDetectionEvaluator.find_save_best_worst(net, saveResultsPath, year, VOC_VAL);
#endif
      , seconds, evaluationCalculationTime); std::cout << evaluationCalculationTime << std::endl;

      break;
    }

  case DEMO:
    {
      Demo::Settings demoSettings;
      demoSettings.debugFlag = true;
      demoSettings.debugWaitTimeMs = 10;
      demoSettings.standardWaitTimeMs = 5;
      demoSettings.saveDir = args.saveDir;

      Capture::Settings captureSettings;
      captureSettings.videoFile = args.videoFile;
      captureSettings.webcamId = 0;
      captureSettings.webcamImageHeight = 480;
      captureSettings.webcamImageWidth = 640;

      Demo cameracapture(demoSettings, captureSettings, detectionSettings);
      if(dataset)
        cameracapture.run(net, dataset->get_category_names(), shapeDescriptorCalculator);
      else
        cameracapture.run(net, VOCDatasetUtil::get_pascal_categories(), shapeDescriptorCalculator);
    }
    break;

  default:
    throw std::runtime_error("No valid mode selected");
  }

  return 0;
}
#endif
