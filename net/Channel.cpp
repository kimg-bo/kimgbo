#include <poll.h>
#include <sstream>
#include <string>
#include "Channel.h"
#include "EventLoop.h"
#include "Logging.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;
	
Channel::Channel(EventLoop* loop, int fd_)
		:m_loop(loop), 
		m_fd(fd_), 
		m_events(0), 
		m_revents(0), 
		m_index(-1), 
		m_logHup(true), 
		m_tied(false), 
		m_eventHandling(false)
{
}

Channel::~Channel()
{
	assert(!m_eventHandling);
}

void Channel::handleEvent(Timestamp recieveTime)
{
	std::tr1::shared_ptr<void> guard;
  if (m_tied)
  {
    guard = m_tie.lock();
    if (guard)
    {
      handleEventWithGuard(recieveTime);
    }
  }
  else
  {
    handleEventWithGuard(recieveTime);
  }
}

void Channel::tie(const std::tr1::shared_ptr<void>& obj)
{
	m_tie = obj;
	m_tied = true;
}
	
std::string Channel::reventsToString() const
{
	std::ostringstream oss;
	oss << m_fd << ": ";
  if (m_revents & POLLIN)
    oss << "IN ";
  if (m_revents & POLLPRI)
    oss << "PRI ";
  if (m_revents & POLLOUT)
    oss << "OUT ";
  if (m_revents & POLLHUP)
    oss << "HUP ";
  if (m_revents & POLLRDHUP)
    oss << "RDHUP ";
  if (m_revents & POLLERR)
    oss << "ERR ";
  if (m_revents & POLLNVAL)
    oss << "NVAL ";

  return oss.str().c_str();
}
	
void Channel::remove()
{
	assert(isNoneEvent());
	m_loop->removeChannel(this);
}
	
void Channel::update()
{
	m_loop->updateChannel(this);
}
	
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
	//LOG_INFO << "Channel::handleEventWithGuard()";
	
	m_eventHandling = true;
	if((m_revents & POLLHUP) && !(m_revents & POLLIN))
	{
		if(m_logHup)
		{
			LOG_WARN << "Channel::handle_event() POLLHUP";
		}
		if(m_closeCallback)
		{
			m_closeCallback();
		}
	}
		
	if(m_revents & POLLNVAL)
	{
		LOG_WARN << "Channel::handle_event() POLLNVAL";
	}
		
	if(m_revents & (POLLERR | POLLNVAL))
	{
		if(m_errorCallback)
		{
			m_errorCallback();
		}
	}
		
	if(m_revents & (POLLIN | POLLPRI | POLLRDHUP))
	{
		if(m_readCallback)
		{
			m_readCallback(receiveTime);
		}
	}
		
	if(m_revents & POLLOUT)
	{
		if(m_writeCallback)
		{
			m_writeCallback();
		}
	}
		
	m_eventHandling = false;
}