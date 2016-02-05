#include <functional>
#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "Logging.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
  : m_baseLoop(baseLoop),
    m_started(false),
    m_numThreads(0),
    m_next(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
  // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
  assert(!m_started);
  m_baseLoop->assertInLoopThread();

  m_started = true;

  for (int i = 0; i < m_numThreads; ++i)
  {
  	std::shared_ptr<EventLoopThread> t(new EventLoopThread(cb));
    m_threads.push_back(t);
    m_loops.push_back(t->startLoop());
  }
  if (m_numThreads == 0 && cb)
  {
    cb(m_baseLoop);
  }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
  m_baseLoop->assertInLoopThread();
  EventLoop* loop = m_baseLoop;

  if (!m_loops.empty())
  {
    // round-robin
    loop = m_loops[m_next];
    ++m_next;
    if (implicit_cast<size_t>(m_next) >= m_loops.size())
    {
      m_next = 0;
    }
  }
  return loop;
}