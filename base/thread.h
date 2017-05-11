#ifndef THREAD_H_
#define THREAD_H_

#include <sys/types.h>
#include <assert.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <iostream>
#include <string>
#include "Atomic.h"

namespace kimgbo
{
//using namespace std;

class Thread
{
public:
	typedef std::function<void ()> threadfunc_;
		
	explicit Thread(const threadfunc_&, const std::string& n = std::string());
	~Thread();
	
	void start();
	int join();
	
	bool started() const { return m_started; }
	pid_t tid() const { return m_tid; }
	//pthread_t tid() const { return m_tid; }
	const std::string& name() const { return m_name; }
		
	static int numCreated() { return s_numCreated.get(); }
	
private:
	static void* startThread(void* thread_obj);
  void runInThread();
	
private:
		bool m_started;
		pthread_t m_pthreadId;
		//pthread_t m_tid;
		pid_t m_tid;
		std::string m_name;
		threadfunc_ m_threadfunc;  //pointer for callback function
		
		static AtomicInt32 s_numCreated;
};

/*inline pid_t gettid()
{
	return static_cast<pid_t>(::syscall(SYS_gettid));
}*/

}

#endif
