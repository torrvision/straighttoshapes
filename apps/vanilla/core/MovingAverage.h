/**
 * vanilla: MovingAverage.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_MOVINGAVERAGE
#define H_VANILLA_MOVINGAVERAGE

#include <tvgutil/containers/CircularBuffer.h>

#include <boost/shared_ptr.hpp>

//#################### TYPEDEFS ####################

template <typename T>
class MovingAverage
{
  //#################### TYPEDEFS ####################
private:
  typedef boost::shared_ptr<tvgutil::CircularBuffer<T> > CircularBuffer_Ptr;

  //#################### PRIVATE VARIABLES ####################
private:
  CircularBuffer_Ptr m_buffer;
  size_t m_length;
  size_t m_currentSize;
  T m_temporarySum;

  //#################### CONSTRUCTORS ####################
public:
  MovingAverage(size_t length, T initialSum)
  : m_buffer(new tvgutil::CircularBuffer<T>(length)),
    m_length(length),
    m_currentSize(0),
    m_temporarySum(initialSum)
  {}

  T push(T element)
  {
    if(m_currentSize == m_length)
    {
      T last = m_buffer->get(1);
      subtract(m_temporarySum, last);
    }
    add(m_temporarySum, element);

    m_buffer->push(element);

    // Increment and clamp.
    ++m_currentSize;
    if(m_currentSize > m_length) m_currentSize = m_length;

    return divide(m_temporarySum, static_cast<T>(m_currentSize));
  }

  void subtract(T& a, T b)
  {
    a -= b;
  }

  void add(T& a, T b)
  {
    a += b;
  }

  T divide(T a, T b)
  {
    return a / b;
  }
};

#endif
