/**
 * tvgutil: CircularBuffer.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016, All rights reserved.
 */

#ifndef H_VANILLA_CIRCULARBUFFER
#define H_VANILLA_CIRCULARBUFFER

#include <vector>

namespace tvgutil{

/**
 * \brieg TODO.
 */
template <typename T>
class CircularBuffer{
  //#################### PRIVATE VARIABLES ####################
private:
  std::vector<T> m_buffer; 

  int m_currentBufferIndex; //position of latest frame 

  size_t m_size;

  //#################### CONSTRUCTORS ####################
public:
  CircularBuffer(size_t size = 1)
  : m_buffer(size),
    m_currentBufferIndex(-1),
    m_size(size)
  {
    if(size < 1) throw std::runtime_error("Cannot create a circular buffer with zero size");
  }

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  void push(const T& data)
  {
    ++m_currentBufferIndex;
    m_currentBufferIndex %= m_size;
    m_buffer[m_currentBufferIndex] = data;
  }

  T get(int offset = 0)
  {
    if(offset == 0)
    {
      return m_buffer[m_currentBufferIndex];
    }
    else
    {
      int ind = (m_currentBufferIndex + offset) % m_size;
      ind = ind < 0 ? ind + m_size : ind;
      return m_buffer[ind];  
    }
  }

  int get_circular_index() const
  {
    return m_currentBufferIndex;
  }

  size_t size() const
  {
    return m_size;
  }
};

}

#endif
