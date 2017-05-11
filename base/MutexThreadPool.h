#ifndef MUTEX_THREADPOOL_H
#define MUTEX_THREADPOOL_H

#include<iostream>
#include<functional>
#include<typeinfo>
#include<string>
#include<vector>
#include<deque>
#include"Mutex.h"
#include"Condition.h"
#include"thread.h"

namespace kimgbo
{

//using namespace std;

class MutexThreadPool
{
public:
	typedef std::function<void ()> Task;
	
	MutexThreadPool(const std::string& name = std::string());
	~MutexThreadPool();
	
	void start(int numbers);
	void stop();
	
	void run(const Task& task);
	
private:
	void runInThread();
	Task take();
	
	MutexLock m_mutex;
	Condition m_cond;
	std::string m_name;
	bool runing;
	std::vector<Thread> m_threads;
	std::deque<Task> m_queue;
};

}

#endif