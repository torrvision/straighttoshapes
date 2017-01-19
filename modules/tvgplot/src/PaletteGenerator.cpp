/**
 * tvgplot: PaletteGenerator.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2015. All rights reserved.
 */

#include "PaletteGenerator.h"

namespace tvgplot {

// TEMPORARY FUNCTIONS
int bitget(int value, int element)
{
  int mask = 1 << element;
  return (value & mask) >> element;
}

int bitshift(int value, int positions)
{
  if(positions >= 0)
    return value << positions;
  else
    return value >> (-1*positions);
}


//#################### PUBLIC STATIC MEMBER FUNCTIONS ####################

std::map<std::string,cv::Scalar> PaletteGenerator::generate_basic_rgba_palette()
{
  const int alpha = 255;

  std::map<std::string,cv::Scalar> result = boost::assign::map_list_of
    ("Black",cv::Scalar(0,0,0,alpha))
    ("White",cv::Scalar(255,255,255,alpha))
    ("Red",cv::Scalar(255,0,0,alpha))
    ("Lime",cv::Scalar(0,255,0,alpha))
    ("Blue",cv::Scalar(0,0,255,alpha))
    ("Yellow",cv::Scalar(255,255,0,alpha))
    ("Cyan",cv::Scalar(0,255,255,alpha))
    ("Magenta",cv::Scalar(255,0,255,alpha))
    ("Silver",cv::Scalar(192,192,192,alpha))
    ("Gray",cv::Scalar(128,128,128,alpha))
    ("Maroon",cv::Scalar(128,0,0,alpha))
    ("Olive",cv::Scalar(128,128,0,alpha))
    ("Green",cv::Scalar(0,128,0,alpha))
    ("Purple",cv::Scalar(128,0,128,alpha))
    ("Teal",cv::Scalar(0,128,128,alpha))
    ("Navy",cv::Scalar(0,0,128,alpha));

  return result;
}

std::map<size_t,cv::Scalar> PaletteGenerator::generate_voc_palette(size_t size)
{
  const int alpha = 255;
  std::map<size_t,cv::Scalar> result;
  for(size_t i = 0; i < size; ++i)
  {
    int id(i);
    int r(0), g(0), b(0);
    for(int j = 0; j < 8; ++j)
    {
      r = r | bitshift(bitget(id, 0), 7 - j);
      g = g | bitshift(bitget(id, 1), 7 - j);
      b = b | bitshift(bitget(id, 2), 7 - j);
      id = bitshift(id, -3);
    }
    result.insert(std::make_pair(i,cv::Scalar(b,g,r,alpha)));
  }
  return result;
}

}
