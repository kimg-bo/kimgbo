#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include<iostream>
#include<set>
#include<vector>
#include "Mutex.h"
#include "Timestamp.h"
#include "Callbacks.h"
#include "Channel.h"

namespace kimgbo
{
	namespace net
	{
		class EventLoop;
		class Timer;
		class TimerId;
		
		class TimerQueue
		{
		public:
			TimerQueue(EventLoop* loop);
			~TimerQueue();
			
			TimerId addTimer(const TimerCallback& cb, Timestamp when, double interval);
			
			void cancel(TimerId timerId);
			
		private:
			// FIXME: use unique_ptr<Timer> instead of raw pointers.
			typedef std::pair<Timestamp, Timer*> Entry;
			typedef std::set<Entry> TimerList;
			typedef std::pair<Timer*, int64_t> ActiveTimer;
			typedef std::set<ActiveTimer> ActiveTimerSet;
				
			void addTimerInLoop(Timer* timer);
			void cancelInLoop(TimerId timerId);
			
			void handleRead();
			
			std::vector<Entry> getExpired(Timestamp now);
			void reset(const std::vector<Entry>& expired, Timestamp now);
				
			bool insert(Timer* timer);
			
			EventLoop* m_loop; //所属EventLoop
			const int m_timerfd; //时间文件描述符
			Channel m_timerfdChannel; //
			TimerList m_timers; //按到期时间的排序列表
			
			ActiveTimerSet m_activeTimers; //按对象地址排序
			bool m_callingExpiredTimers;
			ActiveTimerSet m_cancelingTimers; //保存的是被取消的定时器
		};
	}
}

#endif