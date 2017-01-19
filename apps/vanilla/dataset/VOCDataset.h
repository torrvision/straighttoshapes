/**
 * vanilla: VOCDataset.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2015. All rights reserved.
 */

#ifndef H_VANILLA_VOCDATASET
#define H_VANILLA_VOCDATASET

#include "Dataset.h"

#include <iostream>
#include <list>
#include <map>
#include <vector>

#include <opencv2/core/core.hpp>

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include "../Util.h"
#include "../core/Detection.h"
#include "../data/DataTransformation.h"

#include "VOCAnnotation.h"

class VOCDataset : public Dataset
{
  //#################### PROTECTED MEMBER VARIABLES ####################
protected:
  /** The development kit root directory. */
  std::string m_codeDir;

  //#################### CONSTRUCTORS ####################
public:
  explicit VOCDataset(const std::string& rootDir);

  //#################### DESTRUCTORS ####################
public:
  virtual ~VOCDataset();

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  virtual std::vector<std::string> get_image_paths(VOCYear year, VOCSplit split, VOCImageType imageType, const boost::optional<size_t>& maxPathCount = boost::none, std::vector<std::string> categories = std::vector<std::string>()) const = 0;

  virtual void visualise_annotation(const std::string& imageName) const = 0;

  virtual std::string get_competition_code(VOCSplit vocSplit, bool seenNonVOCData) const = 0;

  friend std::ostream& operator<<(std::ostream& os, const VOCDataset& d);

  //#################### PROTECTED MEMBER FUNCTIONS ####################
protected:
  std::vector<std::string> construct_image_paths_from_split_files(const std::list<std::string>& splitFiles, VOCImageType imageType) const;

  std::list<std::string> get_split_files(VOCYear year, VOCTask task, VOCSplit split, std::vector<std::string> categories = std::vector<std::string>()) const;

  virtual void initialise_annotation() = 0;

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  std::list<std::string> get_split_files(
    const std::vector<std::string>& years,
    const std::string& task,
    const std::vector<std::string>& splits,
    std::vector<std::string> categories = std::vector<std::string>()) const;

  std::list<std::string> find_directories_and_files();
};

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const VOCDataset& d);

//#################### TYPEDEFS ####################

typedef boost::shared_ptr<VOCDataset> VOCDataset_Ptr;
typedef boost::shared_ptr<const VOCDataset> VOCDataset_CPtr;

#endif
