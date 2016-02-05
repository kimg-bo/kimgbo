#include<netinet/in.h>
#include<netinet/tcp.h>
#include<strings.h>
#include"Socket.h"
#include"SocketOps.h"
//#include"InetAddress.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
Socket::~Socket()
{
	sockets::close(m_sockfd);
}
		
void Socket::bindAddress(const InetAddress& localaddr)
{
	sockets::bindOrDie(m_sockfd, localaddr.getSockAddrInet());
}

void Socket::listen()
{
	sockets::listenOrDie(m_sockfd);
}
		
int Socket::accept(InetAddress* peeraddr)
{
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	int connfd = sockets::accept(m_sockfd, &addr);
	if(connfd >= 0)
	{
		peeraddr->setSockAddrInet(addr);
	}
	return connfd;
}

void Socket::shutdownWrite()
{
	sockets::shutdownWrite(m_sockfd);
}

void Socket::setTcpNoDelay(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
  ::setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::setKeepAlive(bool on)
{
	int optval = on ? 1 : 0;
  ::setsockopt(m_sockfd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}