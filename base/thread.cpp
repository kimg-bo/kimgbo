#include <assert.h>
#include <string>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include "CurrentThread.h"
#include "thread.h"
#include "Logging.h"
//#include "../log/log.h"

namespace kimgbo
{
	namespace CurrentThread
	{
  	__thread int t_cachedTid = 0;
  	__thread char t_tidString[32];
  	__thread const char* t_threadName = "unknown";
  	const bool sameType = std::is_same<int, pid_t>::value;
  	static_assert(sameType, "sameType is false!!!");
	}

	namespace detail
	{

		pid_t gettid()
		{
  		return static_cast<pid_t>(::syscall(SYS_gettid));
		}

		void afterFork()
		{
  		kimgbo::CurrentThread::t_cachedTid = 0;
  		kimgbo::CurrentThread::t_threadName = "main";
  		CurrentThread::tid();
  		// no need to call pthread_atfork(NULL, NULL, &afterFork);
		}

		class ThreadNameInitializer
		{
 		public:
  		ThreadNameInitializer()
  		{
    		kimgbo::CurrentThread::t_threadName = "main";
    		CurrentThread::tid();
    		pthread_atfork(NULL, NULL, &afterFork);
  		}
		};

		ThreadNameInitializer init;
	}
}

using namespace kimgbo;

void CurrentThread::cacheTid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = detail::gettid();
    int n = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
    assert(n == 6); (void) n;
  }
}

bool CurrentThread::isMainThread()
{
  return tid() == ::getpid();
}

AtomicInt32 Thread::s_numCreated;

//using namespace kimgbo;

Thread::Thread(const threadfunc_& run_func, const std::string& n)
					:m_started(false), m_pthreadId(0), m_tid(0), m_name(n), m_threadfunc(run_func) 
{
	s_numCreated.increment();
}

Thread::~Thread()
{
	//m_started = false;
}

void* Thread::startThread(void* thread_obj)
{
	Thread* thread_ = static_cast<Thread*>(thread_obj);
	thread_->runInThread();
	return NULL;
}
 
void Thread::runInThread()
{
	m_tid = CurrentThread::tid();
	kimgbo::CurrentThread::t_threadName = m_name.empty() ? "muduoThread" : m_name.c_str();
	try
	{
		m_threadfunc();
	}
	catch(...)
	{
		throw std::exception();
	}
}

void Thread::start()
{
	assert(!m_started);
	m_started = true;
	
	errno = pthread_create(&m_pthreadId, NULL, startThread, this);
	if(0 != errno)
	{
		//log( LOG_ERR, __FILE__, __LINE__, "Failed in pthread_create\n");
		LOG_SYSFATAL << "Failed in pthread_create";
	}
}

int Thread::join()
{
	assert(m_started);
	return pthread_join(m_pthreadId, NULL);
}