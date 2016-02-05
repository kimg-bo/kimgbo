#ifndef TIMERID_H
#define TIMERID_H

namespace kimgbo
{
	namespace net
	{
		class Timer;
		class TimerId
		{
		public:
			TimerId():m_timer(NULL), m_sequence(0)
			{
			}
			
			TimerId(Timer* timer, int64_t seq):m_timer(timer), m_sequence(seq)
			{
			}
			
			friend class TimerQueue;
			
		private:
			Timer* m_timer; //ËùÊô¶¨Ê±Æ÷
			int64_t m_sequence; //ÐòºÅ
		};
	}
}




#endif