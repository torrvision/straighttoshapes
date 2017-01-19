/**
 * vanilla: DarknetUtil.cpp
 * Copyright (c) Torr Vision Group, University of Oxford, 2015, All rights reserved.
 */

#include "DarknetUtil.h"
#include "Util.h"

#include <cstring>

//#################### PUBLIC STATIC MEMBER FUNCTIONS ####################

char ** DarknetUtil::convert_vector_string_to_char_array(const std::vector<std::string>& v)
{
  size_t size = v.size();
  char ** array = new char*[size];
  for(size_t i = 0; i < size; ++i)
  {
    array[i] = new char[v[i].length() + 1];
    std::strcpy(array[i], v[i].c_str());
  }

  return array;
}

std::vector<float> DarknetUtil::predict(network& net, const cv::Mat3b& im)
{
  if(net.w != im.cols || net.h != im.rows)
    throw std::runtime_error("The network is expecting an image of size: " + net.w + 'x' + net.h);

  int netBatch = net.batch;
  if(netBatch > 1) set_batch_network(&net, 1);

  // TODO: this should go in DarknetUtil.
  float *imData = Util::make_rgb_image(im, 1/255.0f);

  float *cpredictions = network_predict(net, imData);

  // size of output should be: gridSideLength * gridSideLength * (boxesPerGridcell * 5 + categoryCount).
  // 5 = |boxParameters| + |confidenceScore|; 4 + 1.
  int predictionSize = get_network_output_size(net);
  // This does a copy;
  std::vector<float> predictions {cpredictions, cpredictions + predictionSize};

  set_batch_network(&net, netBatch);

  delete [] imData;

  return predictions;
}
