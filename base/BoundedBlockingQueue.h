#ifndef BOUNDEDBLOCKINGQUEUE_H
#define BOUNDEDBLOCKINGQUEUE_H

#include<vector>
#include<assert.h>
#include"Mutex.h"
#include"Condition.h"

template<typename T>
class BoundedBlockingQueue
{
public:
	explicit BoundedBlockingQueue(size_t maxSize):m_mutex(), m_capacity(maxSize), m_notEmpty(m_mutex), m_notFull(m_mutex)
	{
		m_queue.reserve(m_capacity);
	}
	
	void put(const T& x)
	{
		{
			MutexLockGuard lock(m_mutex);
			while(m_queue.size() == m_capacity)
			{
				m_notFull.wait();
			}
			assert(!(m_queue.size() == m_capacity));
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
		m_queue.erase(m_queue.begin());
		
		m_notFull.notify();
		return front;
	}
	
	bool empty() const
	{
		MutexLockGuard lock(m_mutex);
		return m_queue.empty();
	}
	
	bool full() const
	{
		MutexLockGuard lock(m_mutex);
		return m_queue.size() == m_capacity;
	}
	
	size_t size() const
	{
		MutexLockGuard lock(m_mutex);
		return m_queue.size();
	}
	
	size_t capacity() const
	{
		MutexLockGuard lock(m_mutex);
		return m_queue.capacity();
	}
	
private:
	mutable MutexLock m_mutex;
	size_t m_capacity;
	Condition m_notEmpty;
	Condition m_notFull;
	std::vector<T> m_queue;
};

#endif