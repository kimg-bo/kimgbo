#ifndef CONDITION_H
#define CONDITION_H

#include<pthread.h>
#include"Mutex.h"

class Condition
{
public:
	explicit Condition(MutexLock& mutex):m_mutex(mutex)
	{
		pthread_cond_init(&m_pcon, NULL);
	}
	
	~Condition()
	{
		pthread_cond_destroy(&m_pcon);
	}
	
	void wait()
	{
		pthread_cond_wait(&m_pcon, m_mutex.getpthreadMutex());
	}
	
	bool waitforSeconds(int seconds);
	
	void notify()
	{
		pthread_cond_signal(&m_pcon);
	}
	
	void notifyAll()
	{
		pthread_cond_broadcast(&m_pcon);
	}
	
private:
	MutexLock& m_mutex;
	pthread_cond_t m_pcon;
};

#endif