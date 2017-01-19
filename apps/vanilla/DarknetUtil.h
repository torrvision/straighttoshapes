/**
 * vanilla: DarknetUtil.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2015, All rights reserved.
 */

#ifndef H_VANILLA_DARKNETUTIL
#define H_VANILLA_DARKNETUTIL

#include <string>
#include <vector>

#include <darknet/network.h>

#include <opencv2/core/core.hpp>

/**
 * \brief This struct provides utility functions.
 */
struct DarknetUtil
{
//#################### PUBLIC STATIC MEMBER FUNCTIONS ####################
static char ** convert_vector_string_to_char_array(const std::vector<std::string>& v);

static std::vector<float> predict(network& net, const cv::Mat3b& im);

//static float train(network& net, const Datum& datum, 
};

#endif
