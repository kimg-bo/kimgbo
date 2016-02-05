#include"Timer.h"
#include"Timestamp.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
AtomicInt64 Timer::s_numCreated;
	
void Timer::restart(Timestamp now)
{
	if(m_repeat)
	{
		m_expiration = addTimer(now, m_interval);
	}
	else
	{
		m_expiration = Timestamp::invalid();
	}
}