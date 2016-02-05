#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <pthread.h>
#include <vector>
#include <functional>
#include <memory>
#include "Mutex.h"
#include "thread.h"
#include "Timestamp.h"
#include "Callbacks.h"
#include "TimerId.h"

namespace kimgbo
{
	namespace net
	{
		class Channel;
		class Poller;
		class TimerQueue;
		
		class EventLoop
		{
		public:
  		typedef std::function<void()> Functor;

  		EventLoop();
  		~EventLoop();  // force out-line dtor, for unique_ptr members.

  		///
  		/// Loops forever.
  		///
  		/// Must be called in the same thread as creation of the object.
  		///
  		void loop();

  		void quit();

  		///
  		/// Time when poll returns, usually means data arrivial.
  		///
  		Timestamp pollReturnTime() const { return m_pollReturnTime; }

  		int64_t iteration() const { return m_iteration; }

  		/// Runs callback immediately in the loop thread.
  		/// It wakes up the loop, and run the cb.
  		/// If in the same loop thread, cb is run within the function.
  		/// Safe to call from other threads.
  		void runInLoop(const Functor& cb);
  		/// Queues callback in the loop thread.
  		/// Runs after finish pooling.
  		/// Safe to call from other threads.
  		void queueInLoop(const Functor& cb);

  		// timers

  		///
  		/// Runs callback at 'time'.
  		/// Safe to call from other threads.
  		///
  		TimerId runAt(const Timestamp& time, const TimerCallback& cb);
  		///
  		/// Runs callback after @c delay seconds.
  		/// Safe to call from other threads.
  		///
  		TimerId runAfter(double delay, const TimerCallback& cb);
  		///
  		/// Runs callback every @c interval seconds.
  		/// Safe to call from other threads.
  		///
  		TimerId runEvery(double interval, const TimerCallback& cb);
  		///
  		/// Cancels the timer.
  		/// Safe to call from other threads.
  		///
  		void cancel(TimerId timerId);

 		 // internal usage
 		 void wakeup();
 		 void updateChannel(Channel* channel);
 		 void removeChannel(Channel* channel);
 		 //bool hasChannel(Channel* channel);
 		 
 		 // pid_t threadId() const { return threadId_; }
  		void assertInLoopThread()
  		{
    		if (!isInLoopThread())
    		{
      		abortNotInLoopThread();
    		}
  		}
  		bool isInLoopThread() const { return m_threadId == kimgbo::CurrentThread::tid(); }
  		// bool callingPendingFunctors() const { return callingPendingFunctors_; }
  		bool eventHandling() const { return m_eventHandling; }

  		static EventLoop* getEventLoopOfCurrentThread();
  		
  	private:
  		void abortNotInLoopThread();
  		void handleRead();  // waked up
  		void doPendingFunctors();

  		void printActiveChannels() const; // DEBUG

  		typedef std::vector<Channel*> ChannelList;

  		bool m_looping; /* atomic */
  		bool m_quit; /* atomic */
  		bool m_eventHandling; /* atomic */
  		bool m_callingPendingFunctors; /* atomic */
  		int64_t m_iteration;
  		const pid_t m_threadId;
  		Timestamp m_pollReturnTime;
  		
  		std::unique_ptr<Poller> m_poller;
  		std::unique_ptr<TimerQueue> m_timerQueue;
  			
  		int m_wakeupFd;
  		// unlike in TimerQueue, which is an internal class,
  		// we don't expose Channel to client.
  		
  		std::unique_ptr<Channel> m_wakeupChannel;
  		
  		ChannelList m_activeChannels;
  		Channel* m_currentActiveChannel;
  		MutexLock m_mutex;
  		std::vector<Functor> m_pendingFunctors; // @BuardedBy mutex_(跨线程调用函数队列)
		};
	}
}

#endif