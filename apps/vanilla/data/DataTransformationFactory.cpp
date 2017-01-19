/**
 * vanilla: DataTransformationFactory.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "DataTransformationFactory.h"

//#################### CONSTRUCTORS ####################
DataTransformationFactory::Settings::Settings()
: maxRotation(4.0f),
  maxTranslation(0.2f),
  maxSpatialScalePercentage(3.0f),
  useRandomFlipping(true),
  maxScaleIntensityPercentage(20.0f),
  maxAddToIntensityValue(10),
  networkImageHeight(448), // FIXME do not hardcode values here.
  networkImageWidth(448)
{}

//#################### CONSTRUCTORS ####################

DataTransformationFactory::DataTransformationFactory(unsigned int seed, const DataTransformationFactory::Settings& settings)
: m_rng(seed),
  m_settings(settings)
{}

DataTransformation DataTransformationFactory::generate_transformation() const
{
  const float smallValue(1e-3);

  float rotationAngle(0.0f);
  if(m_settings.maxRotation > smallValue)
    rotationAngle = m_rng.generate_real_from_uniform(-m_settings.maxRotation, m_settings.maxRotation);

  float xTranslation(0.0f);
  float yTranslation(0.0f);
  if(m_settings.maxTranslation > smallValue)
  {
    xTranslation = m_rng.generate_real_from_uniform(-m_settings.maxTranslation, m_settings.maxTranslation);
    yTranslation = m_rng.generate_real_from_uniform(-m_settings.maxTranslation, m_settings.maxTranslation);
  }

  float spatialScaleFactor(1.0f);
  if(m_settings.maxSpatialScalePercentage > smallValue)
    spatialScaleFactor = 1.0f + (m_rng.generate_real_from_uniform(0.0f, m_settings.maxSpatialScalePercentage * 0.01f)) * (m_rng.generate_int_from_uniform(0, 1)*2-1);

  float scaleIntensityFactor(1.0f);
  if(m_settings.maxScaleIntensityPercentage > smallValue)
    scaleIntensityFactor = 1.0f + (m_rng.generate_real_from_uniform(0.0f, m_settings.maxScaleIntensityPercentage * 0.01f)) * (m_rng.generate_int_from_uniform(0, 1)*2-1);

  float addToIntensityValue(0.0f);
  if(m_settings.maxAddToIntensityValue > smallValue)
    addToIntensityValue = m_rng.generate_real_from_uniform(-m_settings.maxAddToIntensityValue, m_settings.maxAddToIntensityValue);

  bool yflip = false;
  if(m_settings.useRandomFlipping) yflip = static_cast<bool>(m_rng.generate_int_from_uniform(0, 1));

  return DataTransformation(rotationAngle, xTranslation, yTranslation, spatialScaleFactor,
                            scaleIntensityFactor, addToIntensityValue, yflip, m_settings.networkImageWidth, m_settings.networkImageHeight);
}

std::vector<DataTransformation> DataTransformationFactory::generate_transformations(size_t transformationCount) const
{
  std::vector<DataTransformation> result;
  for(size_t i = 0; i < transformationCount; ++i)
  {
    result.push_back(generate_transformation());
  }

  return result;
}

//#################### OUTPUT ####################

#define PRT(X) os << #X << ": " << X << '\n'
std::ostream& operator<<(std::ostream& os, const DataTransformationFactory& d)
{
  PRT(d.m_settings.maxRotation);
  PRT(d.m_settings.maxTranslation);
  PRT(d.m_settings.maxSpatialScalePercentage);
  PRT(d.m_settings.useRandomFlipping);
  PRT(d.m_settings.maxScaleIntensityPercentage);
  PRT(d.m_settings.maxAddToIntensityValue);
  PRT(d.m_settings.networkImageHeight);
  PRT(d.m_settings.networkImageWidth);
  return os;
}
#undef PRT
