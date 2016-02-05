#include<stdio.h>
#include"../Poller.h"
#include"PollPoller.h"
#include"EPollPoller.h"

using namespace kimgbo::net;
	
Poller* Poller::newDefaultPoller(EventLoop* loop)
{
	if(::getenv("MUDUO_USE_POLL"))
	{
		return new PollPoller(loop);
	}
	else
	{
		return new EPollPoller(loop);
	}
}