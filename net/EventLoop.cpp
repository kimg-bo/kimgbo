#include <sys/eventfd.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <functional>
#include <algorithm> 
#include "EventLoop.h"
#include "Logging.h"
#include "Mutex.h"
#include "Singleton.h"
#include "Channel.h"
#include "Poller.h"
#include "SocketOps.h"
#include "TimerQueue.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
namespace 
{
__thread EventLoop* t_loopInThisThread = 0;

const int kPollTimeMs = 10000;

int createEventfd()
{
	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
    LOG_SYSERR << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
 public:
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN);
    LOG_TRACE << "Ignore SIGPIPE";
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
	
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
	return t_loopInThisThread;
}

EventLoop::EventLoop()
	  : m_looping(false),
    m_quit(false),
    m_eventHandling(false),
    m_callingPendingFunctors(false),
    m_iteration(0),
    m_threadId(kimgbo::CurrentThread::tid()),
    m_poller(Poller::newDefaultPoller(this)),
    m_timerQueue(new TimerQueue(this)),
    m_wakeupFd(createEventfd()),
    m_wakeupChannel(new Channel(this, m_wakeupFd)),
    m_currentActiveChannel(NULL)
{
	LOG_TRACE << "EventLoop created " << this << " in thread " << m_threadId;
	
	if(t_loopInThisThread)
	{
		LOG_FATAL << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << m_threadId;
	}
	else
  {
    t_loopInThisThread = this;
  }
  m_wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
  m_wakeupChannel->enableReading();
}
  		
EventLoop::~EventLoop()
{
	::close(m_wakeupFd);
	t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
	assert(!m_looping);
	assertInLoopThread();
	m_looping = true;
	m_quit = false;
	LOG_TRACE << "EventLoop " << this << " start looping";
	
	while(!m_quit)
	{
		m_activeChannels.clear();
		m_pollReturnTime = m_poller->poll(kPollTimeMs, &m_activeChannels);
		++m_iteration;
		
		if (Logger::logLevel() <= Logger::DEBUG)
    {
      printActiveChannels();
    }
    
    m_eventHandling = true;
    for (ChannelList::iterator it = m_activeChannels.begin();
        it != m_activeChannels.end(); ++it)
    {
      m_currentActiveChannel = *it;
      m_currentActiveChannel->handleEvent(m_pollReturnTime);
    }
    m_currentActiveChannel = NULL;
    m_eventHandling = false;
    doPendingFunctors();
	}
	LOG_TRACE << "EventLoop " << this << " stop looping";
  m_looping = false;
}

void EventLoop::quit()
{
	m_quit = true;
  if (!isInLoopThread())
  {
    wakeup();
  }
}

void EventLoop::runInLoop(const Functor& cb)
{
	if (isInLoopThread())
  {
    cb();
  }
  else
  {
    queueInLoop(cb);
  }
}

void EventLoop::queueInLoop(const Functor& cb)
{
	{
  	MutexLockGuard lock(m_mutex);
  	m_pendingFunctors.push_back(cb);
  }

  if (!isInLoopThread() || m_callingPendingFunctors)
  {
    wakeup();
  }
}

TimerId EventLoop::runAt(const Timestamp& time_, const TimerCallback& cb)
{
	return m_timerQueue->addTimer(cb, time_, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
	Timestamp time_(kimgbo::addTimer(Timestamp::now(), delay));
		
  return runAt(time_, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
	Timestamp time_(addTimer(Timestamp::now(), interval));
  return m_timerQueue->addTimer(cb, time_, interval);
}

void EventLoop::cancel(TimerId timerId)
{
	return m_timerQueue->cancel(timerId);
}

void EventLoop::wakeup()
{
	uint64_t one = 1;
  ssize_t n = sockets::write(m_wakeupFd, &one, sizeof(one));
  if (n != sizeof(one))
  {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}
 		 
void EventLoop::updateChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
  assertInLoopThread();
  m_poller->updateChannel(channel);
}
 		 
void EventLoop::removeChannel(Channel* channel)
{
	assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (m_eventHandling)
  {
  	assert(m_currentActiveChannel == channel || std::find(m_activeChannels.begin(), m_activeChannels.end(), channel) == m_activeChannels.end());
  }
  m_poller->removeChannel(channel);
}

/*bool EventLoop::hasChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  return m_poller->hasChannel(channel);
}*/
 		 
void EventLoop::abortNotInLoopThread()
{
	 LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this << " was created in threadId_ = " << m_threadId << ", current thread id = " <<  CurrentThread::tid();
}
  		
void EventLoop::handleRead()
{
	uint64_t one = 1;
  ssize_t n = sockets::read(m_wakeupFd, &one, sizeof(one));
  if (n != sizeof one)
  {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}
  		
void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
  m_callingPendingFunctors = true;

  {
  MutexLockGuard lock(m_mutex);
  functors.swap(m_pendingFunctors);
  }

  for (size_t i = 0; i < functors.size(); ++i)
  {
    functors[i]();
  }
  m_callingPendingFunctors = false;
}

void EventLoop::printActiveChannels() const
{
	for (ChannelList::const_iterator it = m_activeChannels.begin(); it != m_activeChannels.end(); ++it)
  {
    const Channel* ch = *it;
    LOG_TRACE << "{" << ch->reventsToString() << "} ";
  }
}