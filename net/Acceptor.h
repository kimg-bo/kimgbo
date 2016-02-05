#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include<functional>
#include "Channel.h"
#include "Socket.h"

namespace kimgbo
{
	namespace net
	{
		class EventLoop;
		class InetAddress;
		
		class Acceptor
		{
		public:
			typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;
			
			Acceptor(EventLoop* loop, const InetAddress& listenAddr);
			~Acceptor();
			
			void setNewConnectionCallback(const NewConnectionCallback& cb)
			{
				m_newConnectionCallback = cb;
			}
			
			bool listenning() const { return m_listenning; }
			void listen();
			
		private:
			void handleRead();
			
			EventLoop* m_loop;
			Socket m_acceptSocket;
			Channel m_acceptChannel;
			NewConnectionCallback m_newConnectionCallback;
			bool m_listenning;
			int m_idleFd;
		};
	}
}

#endif