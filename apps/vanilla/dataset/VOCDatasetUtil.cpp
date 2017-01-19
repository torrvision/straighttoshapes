#include "VOCDatasetUtil.h"

#include <boost/assign/list_of.hpp>

std::vector<std::string> VOCDatasetUtil::get_pascal_categories()
{
  return boost::assign::list_of<std::string>("aeroplane")("bicycle")("bird")("boat")("bottle")("bus")("car")("cat")("chair")("cow")("diningtable")("dog")("horse")("motorbike")("person")("pottedplant")("sheep")("sofa")("train")("tvmonitor");
}
