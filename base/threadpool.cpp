#include<assert.h>
#include<stdio.h>
#include<string.h>
#include<iostream>
#include<string>
#include<functional>
#include<typeinfo>
#include"threadpool.h"

using namespace kimgbo;
typedef std::function<void ()> Task;

ThreadPool::ThreadPool(const std::string& name):m_mutex(), m_cond(m_mutex), m_name(name), runing(false)
{
}

ThreadPool::~ThreadPool()
{
	if(runing)
	{
		stop();
	}
}

void ThreadPool::start(int numbers)
{
	assert(m_threads.empty());
	runing = true;
	
	m_threads.reserve(numbers);
	for(int i=0; i<numbers; i++)
	{
		char id[32];
		memset(id, '\0', sizeof(id));
		snprintf(id, sizeof(id), "%d", i);
		Thread thread_(std::bind(&ThreadPool::runInThread, this), id);
		
		m_threads.push_back(thread_);
		m_threads[i].start();
	}
}

void ThreadPool::stop()
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
		
void ThreadPool::run(const Task& task)
{
	if(m_queue.empty())
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

Task ThreadPool::take()
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
	
void ThreadPool::runInThread()
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
	
