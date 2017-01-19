/**
 * tvgshape: AEShapeDescriptorCalculator.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#include "AEShapeDescriptorCalculator.h"

#include <cassert>
#include <stdexcept>

#include "ShapeDescriptorUtil.h"

extern "C"
{
#include <lauxlib.h>
}

namespace tvgshape {

//#################### CONSTRUCTORS ####################

AEShapeDescriptorCalculator::AEShapeDescriptorCalculator(lua_State *L, const std::string& scriptFilename, const std::string& modelDir, int descriptorSize, int binaryMaskThreshold)
: m_binaryMaskThreshold(binaryMaskThreshold), m_descriptorSize(descriptorSize), m_L(L)
{
  luaL_openlibs(L);
  if(luaL_dofile(L, scriptFilename.c_str()) != 0)
  {
    puts(lua_tostring(L,-1));
    lua_close(L);
    throw std::runtime_error("Could not find Lua script");
  }

  luabridge::LuaRef init = luabridge::getGlobal(L, "init");
  init(descriptorSize, modelDir);
}

//#################### PUBLIC MEMBER FUNCTIONS ####################

std::vector<float> AEShapeDescriptorCalculator::from_mask(const cv::Mat1b& mask, size_t descriptorSize) const
{
  boost::lock_guard<boost::mutex> guard(m_mtx);
  assert(descriptorSize == static_cast<size_t>(m_descriptorSize));

  int k = 64;
  cv::Mat1b resizedMask(k,k);
  cv::resize(mask, resizedMask, cv::Size(k,k), 0.0, 0.0, cv::INTER_AREA);

  std::vector<float> linearisedMask = ShapeDescriptorUtil::make_gray_image(resizedMask, 1/255.0f);

  FVTransformer encode(m_L, "AE_shape_enc");
  return encode(linearisedMask, m_descriptorSize);
}

cv::Mat1b AEShapeDescriptorCalculator::to_mask(const std::vector<float>& descriptor, const cv::Size& maskSize) const
{
  boost::lock_guard<boost::mutex> guard(m_mtx);
  assert(static_cast<int>(descriptor.size()) == m_descriptorSize);

  FVTransformer decode(m_L, "AE_enc_shape");
  std::vector<float> linearisedMask = decode(descriptor, m_descriptorSize);

  cv::Mat1b outputMask = ShapeDescriptorUtil::make_gray_image(linearisedMask, 64, 64, 255.0f);

  cv::Mat1b finalMask;
  cv::resize(outputMask, finalMask, maskSize, 0.0, 0.0, cv::INTER_CUBIC);
  cv::threshold(finalMask, finalMask, m_binaryMaskThreshold, 255, cv::THRESH_BINARY);
  return finalMask;
}

}
