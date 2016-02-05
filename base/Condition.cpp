#include <time.h>
#include <errno.h>
#include "Condition.h"

bool Condition::waitforSeconds(int seconds)
{
	struct timespec abstime;
  clock_gettime(CLOCK_REALTIME, &abstime);
  abstime.tv_sec += seconds;
  return ETIMEDOUT == pthread_cond_timedwait(&m_pcon, m_mutex.getpthreadMutex(), &abstime);
}