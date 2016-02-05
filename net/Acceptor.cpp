#include <errno.h>
#include <fcntl.h>
#include<functional>
#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "SocketOps.h"
#include "Logging.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
	:m_loop(loop), 
	m_acceptSocket(sockets::createNonblockingOrDie()), 
	m_acceptChannel(loop, m_acceptSocket.fd()), 
	m_listenning(false), 
	m_idleFd(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
	assert(m_idleFd > 0);
	m_acceptSocket.setReuseAddr(true);
	m_acceptSocket.bindAddress(listenAddr);
	m_acceptChannel.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
	m_acceptChannel.disableAll();
	m_acceptChannel.remove();
	::close(m_idleFd);
}

void Acceptor::listen()
{
	m_loop->assertInLoopThread();
	m_listenning = true;
	m_acceptSocket.listen();
	m_acceptChannel.enableReading();
}

void Acceptor::handleRead()
{
	LOG_INFO << "Acceptor::handleRead()";
	
	m_loop->assertInLoopThread();
	InetAddress peerAddr(0);
	
	int connfd = m_acceptSocket.accept(&peerAddr);
	if(connfd >= 0)
	{
		if(m_newConnectionCallback)
		{
			m_newConnectionCallback(connfd, peerAddr);
		}
		else
		{
			sockets::close(connfd);
		}
	}
	else
	{
		if(errno == EMFILE)
		{
			::close(m_idleFd);
			m_idleFd = ::accept(m_acceptSocket.fd(), NULL, NULL);
			::close(m_idleFd);
			m_idleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
		}
	}
}