#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <tvgutil/containers/CircularQueue.h>
using namespace tvgutil;

typedef boost::shared_ptr<CircularQueue<int> > CQ_Ptr;

class DataLoader
{
  typedef boost::shared_ptr<CircularQueue<int> > CQ_Ptr;

private:
  int m_loadAndProcessTime;
  CQ_Ptr m_cq;

public:
  DataLoader(int loadAndProcessTime, CQ_Ptr& cq)
  : m_loadAndProcessTime(loadAndProcessTime),
    m_cq(cq)
  {
    // Initialise the first element on construction.
    if(!m_cq->push(0)) throw std::runtime_error("Something wrong");
    //std::cout << "loaded: " << 0 << std::endl;
  }

public:
  void load_loop(int maxDataSamples)
  {
    for(int i = 1; i < maxDataSamples; ++i)
    {
      sleep(m_loadAndProcessTime);
      
      bool loaded = false;
      while(!loaded)
      {
        loaded = m_cq->push(i);
      }
      //std::cout << "loaded: " << i << std::endl;
    }
  }
};

class DataProcessor
{
  public:
  static void process(int processingTime, CQ_Ptr& cq)
  {
    while(true)
    {
      boost::optional<const int&> x = cq->pop();
      if(x)
      {
        //std::cout << "processing: " << *x << std::endl;
        sleep(processingTime);
        break;
      }
    }
  }
};

BOOST_AUTO_TEST_SUITE(test_CircularQueue)

BOOST_AUTO_TEST_CASE(fill_empty_test)
{
  CircularQueue<int> cq(5);
  for(size_t i = 0; i < 10; ++i)
  {
    cq.push(i);
  }
  BOOST_CHECK_EQUAL(cq.full(), true);

  for(size_t i = 0; i < 10; ++i)
  {
    boost::optional<const int&> x = cq.pop();
    if(x)
    {
      BOOST_CHECK_EQUAL(*x, i);
    }
    else
    {
      BOOST_CHECK_EQUAL(cq.empty(), true);
    }
  }
}

BOOST_AUTO_TEST_CASE(app_test)
{
  CQ_Ptr dataBuffer(new CircularQueue<int>(5));

  const int dataLoadTime(0.01);
  DataLoader dataLoader(dataLoadTime, dataBuffer);

  const int maxDataSamples(10);
  boost::thread dataLoadingThread(&DataLoader::load_loop, boost::ref(dataLoader), maxDataSamples);

  const int dataProcessTime(0.02);
  for(int i = 0; i < maxDataSamples; ++i)
  {
    DataProcessor::process(dataProcessTime, dataBuffer);
  }

  dataLoadingThread.join();

  BOOST_CHECK_EQUAL(dataBuffer->empty(), true);
}

BOOST_AUTO_TEST_SUITE_END()
