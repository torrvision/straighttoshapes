/**
 * vanilla: Util.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2015, All rights reserved.
 */

#ifndef H_VANILLA_UTIL
#define H_VANILLA_UTIL

#include "core/VOCBox.h"

#include <set>
#include <boost/unordered_map.hpp>

#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>

#include <tvgutil/numbers/NumberSequenceGenerator.h>

#include <opencv2/core/core.hpp>

struct Vec3bHash
{
  size_t operator() (const cv::Vec3b& k) const
  {
    return 1e6*k.val[0] + 1e3*k.val[1] + k.val[2];
  }
};

/*
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
  for(typename std::vector<T>::const_iterator it = v.cbegin(), iend = v.cend(); it != iend; ++it)
  {
    os << *it << '\n';
  }

  return os;
}
*/


/**
 * \brief This struct provides utility functions.
 */
struct Util
{
//#################### PUBLIC STATIC MEMBER FUNCTIONS ####################

template <typename T>
static T average_vector(const std::vector<T>& v)
{
  return std::accumulate(v.begin(), v.end(), static_cast<T>(0.0))/static_cast<T>(v.size());
}

template <typename T>
static std::vector<T> to_vector(const boost::unordered_map<size_t, T>& hashmap)
{
  std::vector<T> result;
  for(typename boost::unordered_map<size_t, T>::const_iterator it = hashmap.begin(), iend = hashmap.end(); it != iend; ++it)
  {
    result.push_back(it->second);
  }

  return result;
}

template <typename T>
static boost::unordered_map<size_t, T> to_hash(const std::vector<T>& v)
{
  boost::unordered_map<size_t, T> hashmap;
  for(size_t i = 0; i < v.size(); ++i)
  {
    const T& element = v[i];
    hashmap.insert(std::make_pair(element.id, element));
  }

  return hashmap;
}

template <typename T>
static void decimate_vector(std::vector<T>& v, size_t level)
{
  size_t counter(0);
  typename std::vector<T>::iterator it = v.begin();
  while(it != v.end())
  {
    if((counter % level) != 0)
    {
      it = v.erase(it);
    }
    else
    {
      ++it;
    }
    ++counter;
  }
}

/**
 * \brief Makes an RGB image of the specified size from some pixel data.
 *
 * \param rgbData The pixel data for the image, in the format [R1,R2,R3, ... , G1,G2,G3, ... , B1,B2,B3, ...].
 * \param width   The width of the image.
 * \param height  The height of the image.
 * \param scaleFactor  The factor by which to scale the image pixels.
 * \return        The image.
 */
static cv::Mat3b make_rgb_image(const float *rgbData, int width, int height, float scaleFactor);
static cv::Mat1b make_gray_image(const float *grayData, int width, int height, float scaleFactor);
static float *make_rgb_image(const cv::Mat3b& im, float scaleFactor);
static float *make_gray_image(const cv::Mat1b& im, float scaleFactor);


template <typename T>
static void resize_vector(std::vector<T>& v, size_t level)
{
  std::vector<size_t> indices = tvgutil::NumberSequenceGenerator::generate_power_steps<size_t>(0, level, v.size());

  if((v.size() > indices[indices.size()-1]) != 0)
  {
    indices.push_back(v.size());
  }

  float average(0.0f);
  size_t indicesCounter(0);
  for(size_t i = 0, size = v.size(); i < size; ++i)
  {
    if(i == indices[indicesCounter])
    {
      average = 0.0f;
      ++indicesCounter;
    }
    average += (v[i] / (indices[indicesCounter] - indices[indicesCounter-1]));

    if(i == (indices[indicesCounter]-1))
    {
      v[indicesCounter-1] = average;
    }
  }

  v.resize(indices.size() - 1);
}

template <typename T>
static std::vector<T> generate_one_hot(size_t id, size_t size)
{
  if(id >= size) throw std::runtime_error("out of bounds in one hot vector generation");

  std::vector<T> result(size,0);
  result[id] = static_cast<T>(1);
  return result;
}

static boost::filesystem::path resources_dir();

static VOCBox mask_to_vocbox(const cv::Mat1b& mask);
static cv::Rect to_rect(const VOCBox& vbox);
static cv::Mat3b colourise_mask(const cv::Mat1b& mask, const cv::Scalar& colour);
static std::set<uint8_t> push_pixels_into_set(const cv::Mat1b& im);
static std::vector<cv::Mat1b> segment_ids_to_binary_mask_channels(const cv::Mat1b& segmentIds);
static std::vector<cv::Mat1b> unique_segments_to_binary_masks(const cv::Mat1b& segmentIds, const std::set<uint8_t>& idsToIgnore);
static cv::Mat1b convert_colourmap_to_category(const cv::Mat3b& colourmapImage, const boost::unordered_map<cv::Vec3b,size_t,Vec3bHash>& colourToCategoryIdHash);
static cv::Mat3b convert_category_to_colourmap(const cv::Mat1b& categoryImage, const std::map<size_t,cv::Vec3b>& categoryIdToColour);
static std::vector<std::vector<cv::Point> > to_contours(const std::vector<std::vector<float> >& polygons);
};

#endif
