#include"Poller.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
Poller::Poller(EventLoop* loop):m_ownerLoop(loop)
{
}

Poller::~Poller()
{
}