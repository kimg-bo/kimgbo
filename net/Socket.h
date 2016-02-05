#ifndef SOCKET_H
#define SOCKET_H

#include"InetAddress.h"

using namespace kimgbo::net;

namespace kimgbo
{
	//class InetAddress;
	
	class Socket
	{
	public:
		explicit Socket(int sockfd):m_sockfd(sockfd)
		{
		}
		
		~Socket();
		
		int fd() const { return m_sockfd; }
		void bindAddress(const InetAddress& localaddr);
		void listen();
		
		int accept(InetAddress* peeraddr);
		void shutdownWrite();
		void setTcpNoDelay(bool on);
		void setReuseAddr(bool on);
		void setKeepAlive(bool on);
		
	private:
		const int m_sockfd;
	};
}

#endif