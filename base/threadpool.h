#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <iostream>
#include <memory>
#include <functional>
#include <typeinfo>
#include <string>
#include <vector>
#include <deque>
#include <time.h>  
#include <sys/time.h> 
#include "Mutex.h"
#include "Condition.h"
#include "thread.h"
#include "Atomic.h"
#include "lockfree/concurrentqueue.h"

namespace kimgbo
{

//using namespace std;

class ThreadPool
{
public:
	typedef std::function<void ()> Task;
	
	ThreadPool(bool m_multipQueue, int queueMaxSize, int alarmSize, int concurrentGrading, int threadNum, 
	int usecSleep, const std::string& name = std::string());
		
	ThreadPool(int threadNum, const std::string& name = std::string());
		
	~ThreadPool();
	
	void start();
	void stop();
	size_t queueSize();
	
	void run(const Task& task);
	
private:
	void runInThread();
	Task take();
	Task pollingTake();
	Task lockFreeTake();
	void init();
	void selectSleep();
	
	volatile bool runing;
	bool m_multipQueue;
	
	unsigned int m_queueMaxSize;
	unsigned int m_alarmSize;
	unsigned int m_concurrentGrading;
	int m_threadNum;
	int m_usecSleep;
	std::string m_name;
	
	detail::AtomicIntegerT<unsigned int> m_queueCurSize;
	detail::AtomicIntegerT<unsigned int> m_pushIndex;
	detail::AtomicIntegerT<unsigned int> m_pullIndex;
	
	std::vector<std::shared_ptr<Thread>> m_threads;
	std::vector<moodycamel::ConcurrentQueue<Task>> m_queues;
	moodycamel::ConcurrentQueue<Task> m_singleQueue;
	
	struct timeval delay;
};

}

#endif