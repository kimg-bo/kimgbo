#include<assert.h>
#include<stdio.h>
#include<string.h>
#include<iostream>
#include<string>
#include<functional>
#include<typeinfo>
#include <errno.h>
#include"threadpool.h"
#include "Logging.h"

using namespace kimgbo;

typedef std::function<void ()> Task;

ThreadPool::ThreadPool(const bool multipQueue, int queueMaxSize, int alarmSize, int concurrentGrading,int threadNum,
	 int usecSleep, const std::string& name): runing(false), m_multipQueue(multipQueue), m_queueMaxSize(queueMaxSize),
	 	 m_alarmSize(alarmSize), m_concurrentGrading(concurrentGrading), m_threadNum(threadNum), m_usecSleep(usecSleep), m_name(name), 
	 	 m_queueCurSize(), m_pushIndex(), m_pullIndex()
{
}

ThreadPool::ThreadPool(int threadNum, const std::string& name): runing(false), m_multipQueue(false), 
	m_threadNum(threadNum), m_name(name)
{	  
}

ThreadPool::~ThreadPool()
{
	if(runing)
	{
		stop();
	}
}

void ThreadPool::start()
{
	assert(m_threads.empty());
	assert(m_queues.empty());
	assert(m_threadNum > 0);
	
	init();
	
	m_threads.reserve(m_threadNum);
	for(int i=0; i<m_threadNum; i++)
	{
		char id[32];
		memset(id, '\0', sizeof(id));
		snprintf(id, sizeof(id), "%d", i);
		
		std::shared_ptr<Thread> t(new Thread(std::bind(&ThreadPool::runInThread, this), id));
		
		m_threads.push_back(t);
		t->start();
	}
}

void ThreadPool::stop()
{
	runing = false;
	for(int i=0; i<m_threadNum; i++)
	{
		m_threads[i]->join();
	}
}

size_t ThreadPool::queueSize()
{
	if(!m_multipQueue)
	{
		return m_singleQueue.size_approx();
	}
	else
	{
		return m_queueCurSize.get();
	}
}

void ThreadPool::run(const Task& task)
{
	if(!m_multipQueue)
	{
		if(m_threads.empty())
		{
			task();
		}
		else
		{
			while(!m_singleQueue.enqueue(task));
		}
	}
	else
	{
		if(m_queueCurSize.get() < m_queueMaxSize)
		{
			if(m_queueCurSize.get() > m_alarmSize)
			{
				LOG_WARN << "ThreadPool Alarm, Name:" << m_name << " CurSize: " << m_queueCurSize.get() << " MaxSize: " << m_queueMaxSize;
				while(!m_queues[m_pushIndex.getAndAdd(1) % m_concurrentGrading].enqueue(task));
			}
			else
			{
				while(!m_queues[m_pushIndex.getAndAdd(1) % m_concurrentGrading].enqueue(task));
			}
			m_queueCurSize.incrementAndGet();
		}
		else
		{
			LOG_ERROR << "ThreadPool Overload, Name:" << m_name << " CurSize: " << m_queueCurSize.get() << " MaxSize: " << m_queueMaxSize;
		}
	}
}

Task ThreadPool::take()
{
	Task task;
	if(m_multipQueue)
	{
		task = pollingTake();
	}
	else
	{
		task = lockFreeTake();
	}
	return task;
}

Task ThreadPool::pollingTake()
{
	Task task;
	while(!m_queues[m_pullIndex.getAndAdd(1) % m_concurrentGrading].try_dequeue(task) && runing)
	{
  	if(m_queueCurSize.get() < 1)
    {
    	selectSleep();
      continue;
    }
	}
	m_queueCurSize.decrementAndGet();
	return task;
}

Task ThreadPool::lockFreeTake()
{
	Task task;
	while(!m_singleQueue.try_dequeue(task) && runing);
	
	return task;
}

void ThreadPool::init()
{
	if(m_multipQueue)
	{
		m_queues.reserve(m_concurrentGrading);
		for(unsigned int i=0; i<m_concurrentGrading; i++)
		{
			m_queues.emplace_back(moodycamel::ConcurrentQueue<Task>());
		}
	}
	runing = true;
	
	delay.tv_sec = 0;
  delay.tv_usec = m_usecSleep;
}

void ThreadPool::selectSleep()
{	
	//select本省会有延迟，默认休眠0微妙，
	//大概的延迟是5微妙，所以这里实际默认的阻塞是5微妙
  select(0, NULL, NULL, NULL, &delay);
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
	
