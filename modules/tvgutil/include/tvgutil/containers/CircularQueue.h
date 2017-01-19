/**
 * vanilla: CircularQueue.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016, All rights reserved.
 */

#ifndef H_VANILLA_CIRCULARQUEUE
#define H_VANILLA_CIRCULARQUEUE

#include <boost/optional.hpp>
#include <boost/thread.hpp>

namespace tvgutil {

/**
 * \brief TODO.
 */
template <typename T>
class CircularQueue
{
  //#################### PRIVATE VARIABLES ####################
private:
  int m_backIndex;

  std::vector<T> m_buffer;

  bool m_empty;

  bool m_full;

  boost::mutex m_mtx;

  int m_nextFreeIndex;

  size_t m_size;

  //#################### CONSTRUCTORS ####################
public:
  CircularQueue(size_t size = 1)
  : m_backIndex(0),
    m_buffer(size),
    m_empty(true),
    m_full(false),
    m_nextFreeIndex(0),
    m_size(size)
  {
    if(size < 1) throw std::runtime_error("Cannot create a circular queue with zero size");
  }

  //#################### PUBLIC MEMBER FUNCTIONS ####################
public:
  bool empty()
  {
    return m_empty;
  }

  bool full()
  {
    return m_full;
  }

  bool push(const T& data)
  {
    boost::lock_guard<boost::mutex> guard(m_mtx);
    if(m_empty || !m_full)
    {
#if 0
      std::cout << "pushed" << std::endl;
      //std::cout << "push: " << data << std::endl;
#endif

      m_buffer[m_nextFreeIndex] = data;
      m_empty = false;

      increment(m_nextFreeIndex);
      // Always leave a gap of one between read and write to avoid writing when memory is still being read.
      if(next_index(m_nextFreeIndex) == m_backIndex)
      {
        m_full = true;
      }

      return true;
    }
    else
    {
      return false;
    }
  }

  boost::optional<const T&> pop()
  {
    boost::lock_guard<boost::mutex> guard(m_mtx);
    if(m_empty)
    {
      return boost::none;
    }
    else
    {
#if 0
      std::cout << "popped" << std::endl;
      //std::cout << "pop: " << m_buffer[m_backIndex] << std::endl;
#endif

      int currentIndex = m_backIndex;
      m_full = false;

      increment(m_backIndex);
      if(m_backIndex == m_nextFreeIndex)
      {
        m_empty = true;
      }

      return m_buffer[currentIndex];
    }
  }

  //#################### PRIVATE MEMBER FUNCTIONS ####################
private:
  void increment(int& index)
  {
    ++index;
    index %= m_size;
  }

  int next_index(int index)
  {
    ++index;
    return index % m_size;
  }
};

#if 0
std::ostream& operator<<(std::ostream& os, const CircularQueue<int>& cq)
{
  for(size_t i = 0, size = cq.m_buffer.size(); i < size; ++i)
  {
    os << cq.m_buffer[i] << ' ';
  }
  os << '\n';
  os << "nextFreeIndex:" <<  cq.m_nextFreeIndex << '\n';
  os << "backIndex" <<  cq.m_backIndex << '\n';
  os << "isEmpty" <<  cq.m_empty << '\n';
  os << "isFull" <<  cq.m_full << '\n';
  os << "size" <<  cq.m_size << '\n';
  return os;
}
#endif

}

#endif
