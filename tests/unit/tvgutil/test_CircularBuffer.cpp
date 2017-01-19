#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <tvgutil/containers/CircularBuffer.h>
using namespace tvgutil;

BOOST_AUTO_TEST_SUITE(test_CircularBuffer)

BOOST_AUTO_TEST_CASE(fill_test)
{
  CircularBuffer<int> cb(5);
  for(size_t i = 0; i < 10; ++i)
  {
    cb.push(i);
    BOOST_CHECK_EQUAL(cb.get(), i);
  }
  BOOST_CHECK_EQUAL(cb.get_circular_index(), 4);
}

BOOST_AUTO_TEST_SUITE_END()
