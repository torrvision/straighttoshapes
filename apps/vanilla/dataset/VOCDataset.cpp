/**
 * vanilla: VOCDataset.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2015. All rights reserved.
 */

#include "VOCDataset.h"
#include "VOCDatasetUtil.h"
#include "VOCAnnotation.h"

#include "../DetectionUtil.h"

#include <fstream>
#include <stdexcept>

#include <boost/assign/list_of.hpp>
#include <boost/filesystem.hpp>

#include <tvgutil/persistence/LineUtil.h>
#include <tvgutil/filesystem/FilesystemUtil.h>
#include <tvgutil/containers/MapUtil.h>
using namespace tvgutil;

#include <tvgplot/PaletteGenerator.h>
using namespace tvgplot;

//#################### CONSTRUCTORS ####################

VOCDataset::VOCDataset(const std::string& rootDir)
: Dataset(rootDir)
{
  m_categories = VOCDatasetUtil::get_pascal_categories();

  for(size_t i = 0, categoryCount = m_categories.size(); i < categoryCount; ++i)
  {
    m_categoryNameToId.insert(std::make_pair(m_categories[i], i));
    m_categoryIdToName.insert(std::make_pair(i, m_categories[i]));
  }

  std::list<std::string> missingPaths = find_directories_and_files();
  output_missing_paths(missingPaths);

  // Initialise the static members of the voc annotation.
  VOCAnnotation::set_category_count(m_categories.size());
  VOCAnnotation::set_category_name_to_id(m_categoryNameToId);
  VOCAnnotation::set_category_id_to_name(m_categoryIdToName);
}

//#################### DESTRUCTORS ####################

VOCDataset::~VOCDataset(){}

//#################### PROTECTED MEMBER FUNCTIONS ####################

std::list<std::string> VOCDataset::get_split_files(VOCYear year, VOCTask task, VOCSplit split, std::vector<std::string> categories) const
{
  std::vector<std::string> years;
  if(year == VOC_2007 || year == VOC_2012)
  {
    years.push_back(m_vocYears[year]);
  }
  else if(year == VOC_ALLYEARS)
  {
    years.push_back(m_vocYears[VOC_2007]);
    years.push_back(m_vocYears[VOC_2012]);
  }

  std::vector<std::string> splits;
  if(split == VOC_TRAIN || split == VOC_TRAINVAL || split == VOC_VAL || split == VOC_TEST)
  {
    splits.push_back(m_splitNames[split]);
  }
  else if(split == VOC_ALLSPLITS)
  {
    splits.push_back(m_splitNames[VOC_TRAIN]);
    splits.push_back(m_splitNames[VOC_TRAINVAL]);
    splits.push_back(m_splitNames[VOC_VAL]);
    splits.push_back(m_splitNames[VOC_TEST]);
  }

  return get_split_files(years, m_vocTasks[task], splits, categories);
}

std::vector<std::string> VOCDataset::construct_image_paths_from_split_files(const std::list<std::string>& splitFiles, VOCImageType imageType) const
{
  std::vector<std::string> imagePaths;
  for(std::list<std::string>::const_iterator it = splitFiles.cbegin(), iend = splitFiles.cend(); it != iend; ++it)
  {
    std::ifstream fs((*it).c_str());
    if(!fs) throw std::runtime_error("Error: The file '" + *it + "'could not be opened");

    std::vector<std::string> imagesInSplit = tvgutil::LineUtil::extract_lines(fs);

    for(size_t j = 0, nameCount = imagesInSplit.size(); j < nameCount; ++j)
    {
      boost::filesystem::path path(*it);
      std::string tmp = path.parent_path().parent_path().parent_path().string() + '/' + m_imageTypeNames[imageType] + '/' + imagesInSplit[j] + MapUtil::lookup(m_imageTypeExtension, m_imageTypeNames[imageType]);
      imagePaths.push_back(tmp);
    }
  }

  return imagePaths;
}

//#################### PRIVATE MEMBER FUNCTIONS ####################

std::list<std::string> VOCDataset::get_split_files(
    const std::vector<std::string>& years,
    const std::string& task,
    const std::vector<std::string>& splits,
    std::vector<std::string> categories) const
{
  std::list<std::string> splitFilePaths;
  for(size_t year = 0, yearCount = years.size(); year < yearCount; ++year)
  {
    for(size_t split = 0, splitCount = splits.size(); split < splitCount; ++split)
    {
      std::string commonPath = m_rootDir + "/VOCdevkit" + "/VOC" + years[year] + "/ImageSets/" + task;
      if(categories.empty())
      {
        std::string splitFile = commonPath + '/' + splits[split] + ".txt";
        splitFilePaths.push_back(splitFile);
      }
      else if(!categories.empty() && (task == "Main") && !(splits[split] == "test"))
      {
        for(size_t category = 0, categoryCount = categories.size(); category < categoryCount; ++category)
        {
          std::string splitFile = commonPath + '/' + categories[category] + '_' + splits[split] + ".txt";
          splitFilePaths.push_back(splitFile);
        }
      }
      else if(!categories.empty() && (task == "Main") && (splits[split] == "test"))
      {
        // Do Nothing.
      }
      else
      {
        throw std::runtime_error("Something is wrong");
      }
    }
  }

  return splitFilePaths;
}

std::list<std::string> VOCDataset::find_directories_and_files()
{
  std::list<std::string> missingPaths;

  std::list<std::string> expectedPaths;
  expectedPaths.push_back(m_rootDir);
  expectedPaths.push_back(m_codeDir = m_rootDir + "/VOCdevkit/VOCcode");
  expectedPaths.push_back(m_rootDir + "/VOCdevkit" + "/VOC" + m_vocYears[VOC_2007]);
  expectedPaths.push_back(m_rootDir + "/VOCdevkit" + "/VOC" + m_vocYears[VOC_2012]);

  /*
  for(size_t year = 0, yearCount = m_vocYears.size(); year < yearCount; ++year)
  {
    expectedPaths.push_back(m_rootDir + "/VOCdevkit" + '/' + m_vocYears[year]);
  }
  */

  missingPaths.splice(missingPaths.end(), FilesystemUtil::get_missing_paths(expectedPaths));

  return missingPaths;
}

//#################### OUTPUT ####################

#define PRT(X) os << #X << ": " << X << '\n'
std::ostream& operator<<(std::ostream& os, const VOCDataset& d)
{
  os << "PASCAL VOC Dataset\n";
  PRT(d.m_rootDir);
  return os;
}
#undef PRT
