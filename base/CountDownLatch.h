#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include"Mutex.h"
#include"Condition.h"

class CountDownLatch
{
public:
	explicit CountDownLatch(int count);
	void wait();
	void countDown();
	int getCount() const;
	
private:
	mutable MutexLock m_mutex;
	Condition m_cond;
	int m_count;
};

#endif