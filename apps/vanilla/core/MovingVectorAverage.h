/**
 * vanilla: MovingVectorAverage.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_VANILLA_MOVINGVECTORAVERAGE
#define H_VANILLA_MOVINGVECTORAVERAGE

#include <iostream>
#include <vector>

#include <tvgutil/containers/CircularBuffer.h>

#include <boost/shared_ptr.hpp>

//#################### TYPEDEFS ####################

template <typename T>
class MovingVectorAverage
{
  //#################### TYPEDEFS ####################
private:
  typedef boost::shared_ptr<tvgutil::CircularBuffer<std::vector<T> > > CircularBuffer_Ptr;

  //#################### PRIVATE VARIABLES ####################
private:
  CircularBuffer_Ptr m_buffer;
  size_t m_length;
  size_t m_currentSize;
  std::vector<T> m_temporarySum;

  //#################### CONSTRUCTORS ####################
public:
  MovingVectorAverage(size_t length, const std::vector<T>& initialSum)
  : m_buffer(new tvgutil::CircularBuffer<std::vector<T> >(length)),
    m_length(length),
    m_currentSize(0),
    m_temporarySum(initialSum)
  {}

  std::vector<T> push(const std::vector<T>& element)
  {
    if(m_temporarySum.size() != element.size()) throw std::runtime_error("Vectors must be of equal size");

    if(m_currentSize == m_length)
    {
      const std::vector<T>& last = m_buffer->get(1);
      subtract(m_temporarySum, last);
    }
    add(m_temporarySum, element);

    m_buffer->push(element);

    // Increment and clamp.
    ++m_currentSize;
    if(m_currentSize > m_length) m_currentSize = m_length;

    return divide(m_temporarySum, static_cast<T>(m_currentSize));
  }

  void subtract(std::vector<T>& a, const std::vector<T>& b)
  {
    std::transform(a.begin(), a.end(), b.begin(), a.begin(), std::minus<T>());
    /*
    for(size_t i = 0; i < a.size(); ++i)
    {
      a[i] -= b[i];
    }
    */
  }

  void add(std::vector<T>& a, const std::vector<T>& b)
  {
    std::transform(a.begin(), a.end(), b.begin(), a.begin(), std::plus<T>());
    /*
    for(size_t i = 0; i < a.size(); ++i)
    {
      a[i] += b[i];
    }
    */
  }

  std::vector<T> divide(const std::vector<T>& a, const T& b)
  {
    std::vector<T> c(a.size());
    std::transform(a.begin(), a.end(), c.begin(), std::bind2nd(std::multiplies<T>(), 1.0/b));

    /*
    for(size_t i = 0; i < a.size(); ++i)
    {
      c[i] = a[i] / b;
    }
    */

    return c;
  }
};

#endif
