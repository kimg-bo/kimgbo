#ifndef EVENTLOOPTHREAD_H
#define EVENTLOOPTHREAD_H

#include <functional>
#include "Condition.h"
#include "Mutex.h"
#include "thread.h"
#include "EventLoop.h"

namespace kimgbo
{
	namespace net
	{
		class EventLoopThread
		{
		public:
			typedef std::function<void(EventLoop*)> ThreadInitCallback;

  		EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback());
  		~EventLoopThread();
  		EventLoop* startLoop(); //启动线程
  
		private:
			void threadFunc(); //线程函数

  		EventLoop* m_loop; //loop指针指向一个EventLoop对象
  		bool m_exiting;
  		Thread m_thread;
  		MutexLock m_mutex;
  		Condition m_cond;
  		ThreadInitCallback m_callback; //若回调不为空，则将会在EventLoop::loop()事件循环之前被调用
		};
	}
}

#endif