#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include <memory>
#include <vector>
#include <functional>
#include "Condition.h"
#include "Mutex.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

namespace kimgbo
{
	namespace net
	{
		//class EventLoop;
		//class EventLoopThread;

		class EventLoopThreadPool
		{
		public:
  		typedef std::function<void(EventLoop*)> ThreadInitCallback;

  		EventLoopThreadPool(EventLoop* baseLoop);
  		~EventLoopThreadPool();
  		void setThreadNum(int numThreads) { m_numThreads = numThreads; }
  		void start(const ThreadInitCallback& cb = ThreadInitCallback());
  		EventLoop* getNextLoop();

 		private:

  		EventLoop* m_baseLoop;
  		bool m_started;
  		int m_numThreads;
  		int m_next;
  		std::vector<std::shared_ptr<EventLoopThread>> m_threads;
  		std::vector<EventLoop*> m_loops;
		};
	}
}


#endif