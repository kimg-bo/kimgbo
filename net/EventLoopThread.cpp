#include "EventLoopThread.h"
#include "EventLoop.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
EventLoopThread::EventLoopThread(const ThreadInitCallback& cb)
  : m_loop(NULL),
    m_exiting(false),
    m_thread(std::bind(&EventLoopThread::threadFunc, this)),
    m_mutex(),
    m_cond(m_mutex),
    m_callback(cb)
{
}

EventLoopThread::~EventLoopThread()
{
  m_exiting = true;
  m_loop->quit(); //终止I/O线程
  m_thread.join();
}

EventLoop* EventLoopThread::startLoop()
{
  assert(!m_thread.started());
  m_thread.start();

  {
    MutexLockGuard lock(m_mutex);
    while (m_loop == NULL)
    {
    	//等待start中的threadfunc创建EventLoop对象成功后才会返回
      m_cond.wait(); 
    }
  }

  return m_loop;
}

void EventLoopThread::threadFunc()
{
	//栈对象，当此函数退出，意味着此I/O线程的终止
  EventLoop loop;

  if (m_callback)
  {
    m_callback(&loop); //有益于和线程本地存储数据结合，提高服务器端的并发性能
  }

  {
    MutexLockGuard lock(m_mutex);
    m_loop = &loop;
    m_cond.notify();
  }

  loop.loop();
  //assert(exiting_);
}