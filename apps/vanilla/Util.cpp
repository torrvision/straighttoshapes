/**
 * vanilla: Util.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2015, All rights reserved.
 */

#include "Util.h"

#include <tvgutil/filesystem/PathFinder.h>
#include <tvgutil/containers/MapUtil.h>
using namespace tvgutil;

//#################### PUBLIC STATIC MEMBER FUNCTIONS ####################

cv::Mat3b Util::make_rgb_image(const float *rgbData, int width, int height, float scaleFactor)
{
  cv::Mat3b result = cv::Mat3b::zeros(height, width);
  int pixelCount = width*height;
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      result(y,x) = cv::Vec3b(
        static_cast<unsigned char>(rgbData[2*pixelCount]*scaleFactor),
        static_cast<unsigned char>(rgbData[1*pixelCount]*scaleFactor),
        static_cast<unsigned char>(rgbData[0*pixelCount]*scaleFactor)
      );
      ++rgbData;
    }
  }
  return result;
}

cv::Mat1b Util::make_gray_image(const float *grayData, int width, int height, float scaleFactor)
{
  cv::Mat1b result = cv::Mat1b::zeros(height, width);
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      float scaledValue = grayData[0] * scaleFactor;
      if(scaledValue > 255.0f) scaledValue = 255.0f;
      if(scaledValue < 0.0f) scaledValue = 0.0f;
      result(y,x) = static_cast<uint8_t>(scaledValue);
      ++grayData;
    }
  }
  return result;
}

float* Util::make_rgb_image(const cv::Mat3b& im, float scaleFactor)
{
  int width = im.cols;
  int height = im.rows;
  int pixelCount = width*height;
  float *rgbData = new float[pixelCount * im.channels()];

  int counter(0);
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      const cv::Vec3b& bgr = im(y,x);
      rgbData[counter + 2*pixelCount] = bgr[0]*scaleFactor;
      rgbData[counter + 1*pixelCount] = bgr[1]*scaleFactor;
      rgbData[counter + 0*pixelCount] = bgr[2]*scaleFactor;
      counter++;
    }
  }

  return rgbData;
}

float* Util::make_gray_image(const cv::Mat1b& im, float scaleFactor)
{
  int width = im.cols;
  int height = im.rows;
  float *grayData = new float[width*height];

  int counter(0);
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      float gray = static_cast<float>(im(y,x));
      grayData[counter] = gray * scaleFactor;
      counter++;
    }
  }

  return grayData;
}

boost::filesystem::path Util::resources_dir()
{
  return find_subdir_from_executable("resources");
}

VOCBox Util::mask_to_vocbox(const cv::Mat1b& mask)
{
  int width = mask.cols;
  int height = mask.rows;

  int xmin = width + 1;
  int xmax = -1;
  int ymin = height + 1;
  int ymax = -1;
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      if(mask(y,x))
      {
        xmin = xmin > x ? x : xmin;
        xmax = xmax < x ? x : xmax;
        ymin = ymin > y ? y : ymin;
        ymax = ymax < y ? y : ymax;
      }
    }
  }

  return VOCBox(xmin, ymin, xmax, ymax);
}

cv::Rect Util::to_rect(const VOCBox& vbox)
{
  return cv::Rect(vbox.xmin, vbox.ymin, vbox.w() - 1, vbox.h() - 1);
}

std::set<uint8_t> Util::push_pixels_into_set(const cv::Mat1b& im)
{
  std::set<uint8_t> result;

  int width = im.cols;
  int height = im.rows;
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      uint8_t id = im(y,x);
      result.insert(id);
    }
  }

  return result;
}

std::vector<cv::Mat1b> Util::segment_ids_to_binary_mask_channels(const cv::Mat1b& segmentIds)
{
  double maxId_;
  // Ignore the void label, FIXME:
  cv::minMaxLoc(segmentIds, NULL, &maxId_, NULL, NULL, segmentIds != 255);
  uint8_t maxId = static_cast<uint8_t>(maxId_);
  std::vector<cv::Mat1b> masks;
  for(uint8_t i = 0; i <= maxId; ++i)
  {
    cv::Mat1b mask = (segmentIds == i);
    int pixelCount = cv::countNonZero(mask);
    if(pixelCount > 0)
    {
      masks.push_back(mask);
    }
    else
    {
      cv::Mat1b dummy;
      masks.push_back(dummy);
    }
  }

  return masks;
}

// FIXME: make me faster!
std::vector<cv::Mat1b> Util::unique_segments_to_binary_masks(const cv::Mat1b& segmentIds, const std::set<uint8_t>& idsToIgnore)
{
  std::set<uint8_t> uniqueIds = push_pixels_into_set(segmentIds);
  std::set<uint8_t> interestingIds;
  std::set_difference(uniqueIds.begin(), uniqueIds.end(), idsToIgnore.begin(), idsToIgnore.end(), std::inserter(interestingIds, interestingIds.end()));

  std::vector<cv::Mat1b> masks(interestingIds.size());
  size_t counter(0);
  for(std::set<uint8_t>::const_iterator it = interestingIds.begin(), iend = interestingIds.end(); it != iend; ++it)
  {
    masks[counter++].push_back(segmentIds == *it);
  }

  return masks;
}

// colour is expected to be in bgra format, range is [0-1].
cv::Mat3b Util::colourise_mask(const cv::Mat1b& mask, const cv::Scalar& colour)
{
  cv::Mat1b blue = mask * colour.val[0];
  cv::Mat1b green = mask * colour.val[1];
  cv::Mat1b red = mask * colour.val[2];

  std::vector<cv::Mat1b> arraysToMerge = {blue, green, red};

  cv::Mat3b colourMask(mask.size());
  cv::merge(arraysToMerge, colourMask);

  return colourMask;
}

cv::Mat1b Util::convert_colourmap_to_category(const cv::Mat3b& colourmapImage, const boost::unordered_map<cv::Vec3b,size_t,Vec3bHash>& colourToCategoryIdHash)
{
  cv::Mat1b categorymapImage(colourmapImage.rows, colourmapImage.cols);

  int width = colourmapImage.cols;
  int height = colourmapImage.rows;
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      boost::unordered_map<cv::Vec3b,size_t,Vec3bHash>::const_iterator it = colourToCategoryIdHash.find(colourmapImage(y,x));
      if(it != colourToCategoryIdHash.end())
      {
        size_t id = it->second;
        if(id < 0 || id > 255) throw std::runtime_error("Category id out of range");
        categorymapImage(y,x) = static_cast<uint8_t>(id);
      }
      else throw std::runtime_error("The inverse colour map does not contain the specified colour");
    }
  }
  return categorymapImage;
}

cv::Mat3b Util::convert_category_to_colourmap(const cv::Mat1b& categoryImage, const std::map<size_t,cv::Vec3b>& categoryIdToColour)
{
  cv::Mat3b colourmapImage(categoryImage.rows, categoryImage.cols);

  int width = colourmapImage.cols;
  int height = colourmapImage.rows;
  for(int y = 0; y < height; ++y)
  {
    for(int x = 0; x < width; ++x)
    {
      uint8_t categoryId = categoryImage(y,x);
      colourmapImage(y,x) = MapUtil::lookup(categoryIdToColour, categoryId);
    }
  }
  return colourmapImage;
}

std::vector<std::vector<cv::Point> > Util::to_contours(const std::vector<std::vector<float> >& polygons)
{
  std::vector<std::vector<cv::Point> > contours(polygons.size());
  for(size_t i = 0; i < polygons.size(); ++i)
  {
    size_t contourSize = polygons[i].size()/2;
    std::vector<cv::Point> contour(contourSize);
    for(size_t j = 0; j < contourSize; ++j)
    {
      size_t xind = j*2;
      size_t yind = xind+1;
      contour[j] = cv::Point(polygons[i][xind],polygons[i][yind]);
    }

    contours[i] = contour;
  }

  return contours;
}
