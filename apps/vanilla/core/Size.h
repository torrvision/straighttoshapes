/**
 * vanilla: Size.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2015. All rights reserved.
 */

#ifndef H_VANILLA_SIZE
#define H_VANILLA_SIZE

#include <ostream>

class Size
{
public:
  int width, height, depth;

  //#################### CONSTRUCTORS #################### 
public:
  Size();
  Size(int width_, int height_, int depth_);
};

//#################### OUTPUT #################### 
std::ostream& operator<<(std::ostream& os, const Size& size);

#endif
