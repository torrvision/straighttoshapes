/**
 * vanilla: DataTransformationFactory.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_DATATRANSFORMATIONFACTORY
#define H_VANILLA_DATATRANSFORMATIONFACTORY

#include "DataTransformation.h"

#include <tvgutil/numbers/RandomNumberGenerator.h>

class DataTransformationFactory
{
public:
  struct Settings
  {
    //~~~~~~~~~~~~~~~~~~~~ PUBLIC VARIABLES ~~~~~~~~~~~~~~~~~~~~
    float maxRotation;
    float maxTranslation;
    float maxSpatialScalePercentage;

    bool useRandomFlipping;

    float maxScaleIntensityPercentage;
    float maxAddToIntensityValue;

    size_t networkImageHeight;
    size_t networkImageWidth;

    //~~~~~~~~~~~~~~~~~~~~ CONSTRUCTORS ~~~~~~~~~~~~~~~~~~~~
    Settings();
  };

  //#################### PRIVATE VARIABLES ####################
private:
  mutable tvgutil::RandomNumberGenerator m_rng;

  Settings m_settings;

  //#################### CONSTRUCTORS ####################
public:
  DataTransformationFactory(unsigned int seed, const DataTransformationFactory::Settings& settings); 

  DataTransformation generate_transformation() const;
  std::vector<DataTransformation> generate_transformations(size_t transformationCount) const;

  friend std::ostream& operator<<(std::ostream& os, const DataTransformationFactory& d);
};

//#################### OUTPUT ####################

std::ostream& operator<<(std::ostream& os, const DataTransformationFactory& d);

#endif


