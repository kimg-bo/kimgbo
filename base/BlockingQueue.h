#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include<deque>
#include<assert.h>
#include"Mutex.h"
#include"Condition.h"

template<typename T>
class BlockingQueue
{
public:
	BlockingQueue():m_mutex(), m_notEmpty(m_mutex), m_queue()
	{
	}
	
	void put(const T& x)
	{
		{
			MutexLockGuard lock(m_mutex);
			m_queue.push_back(x);
		}
		m_notEmpty.notify();
	}
	
	T take()
	{
		MutexLockGuard lock(m_mutex);
		while(m_queue.empty())
		{
			m_notEmpty.wait();
		}
		assert(!m_queue.empty());
		T front(m_queue.front());
		m_queue.pop_front();
		return front;
	}
	
	size_t size()
	{
		MutexLockGuard lock(m_mutex);
		return m_queue.size();
	}
	
private:
	mutable MutexLock m_mutex;
	Condition m_notEmpty;
	std::deque<T> m_queue;
};

#endif