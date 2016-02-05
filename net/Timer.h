#ifndef TIMER_H
#define TIMER_H

#include<stdint.h>
#include "Atomic.h"
#include "Timestamp.h"
#include "Callbacks.h"

namespace kimgbo
{
	namespace net
	{
		class Timer
		{
		public:
			Timer(const TimerCallback& cb, Timestamp when, double interval)
			 : m_callback(cb), 
			 	 m_expiration(when), 
				 m_interval(interval), 
				 m_repeat(interval > 0.0),
				 m_sequence(s_numCreated.incrementAndGet())
			{
			}
			
			void run() const
			{
				m_callback();
			}
			
			Timestamp expiration() const  { return m_expiration; }
  		bool repeat() const { return m_repeat; }
  		int64_t sequence() const { return m_sequence; }
  		
  		void restart(Timestamp now);
  		
  		static int64_t numCreated() { return s_numCreated.get(); }
			
		private:
			const TimerCallback m_callback;
			Timestamp m_expiration; //超时时间
			const double m_interval; //是否是连续的（为0则为一次性定时器）
			const bool m_repeat; //是否重复
			const int64_t m_sequence; //定时器序号
			
			static AtomicInt64 s_numCreated; //已创建定时器个数
		};
	}
}

#endif