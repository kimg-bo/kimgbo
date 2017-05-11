#include<assert.h>
#include<stdio.h>
#include<string.h>
#include<iostream>
#include<string>
#include<functional>
#include<typeinfo>
#include"MutexThreadPool.h"

using namespace kimgbo;
typedef std::function<void ()> Task;

MutexThreadPool::MutexThreadPool(const std::string& name):m_mutex(), m_cond(m_mutex), m_name(name), runing(false)
{
}

MutexThreadPool::~MutexThreadPool()
{
	if(runing)
	{
		stop();
	}
}

void MutexThreadPool::start(int numbers)
{
	assert(m_threads.empty());
	runing = true;
	
	m_threads.reserve(numbers);
	for(int i=0; i<numbers; i++)
	{
		char id[32];
		memset(id, '\0', sizeof(id));
		snprintf(id, sizeof(id), "%d", i);
		Thread thread_(std::bind(&MutexThreadPool::runInThread, this), id);
		
		m_threads.push_back(thread_);
		m_threads[i].start();
	}
}

void MutexThreadPool::stop()
{
	{
		MutexLockGuard lock(m_mutex);
		runing = false;
		m_cond.notifyAll();
	}
	
	for(int i=0; i<m_threads.size(); i++)
	{
		m_threads[i].join();
	}
}
		
void MutexThreadPool::run(const Task& task)
{
	if(m_threads.empty())
	{
		task();
	}
	else
	{
		{
			MutexLockGuard lock(m_mutex);
			m_queue.push_back(task);
		}
		m_cond.notify();
	}
}

Task MutexThreadPool::take()
{
	MutexLockGuard lock(m_mutex);
	
	while(m_queue.empty() && runing)
	{
		m_cond.wait();
	}
	Task task;
	if(!m_queue.empty())
	{
		task = m_queue.front();
		m_queue.pop_front();
	}
	return task;
}
	
void MutexThreadPool::runInThread()
{
	try
	{
		while(runing)
		{
			Task task(take());
			if(task)
			{
				task();
			}
		}
	}
	catch(...)
	{
		throw std::exception();
	}
}
	
