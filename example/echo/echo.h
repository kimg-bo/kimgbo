#ifndef EXAMPLES_ECHO_ECHO_H
#define EXAMPLES_ECHO_ECHO_H

#include "TcpServer.h"

class EchoServer
{
public:
	EchoServer(kimgbo::net::EventLoop* loop, const kimgbo::net::InetAddress& listenAddr);
	
	void start();
	
private:
	void onConnection(const kimgbo::net::TcpConnectionPtr& conn);
		
	void onMessage(const kimgbo::net::TcpConnectionPtr& conn, kimgbo::net::Buffer* buf, kimgbo::Timestamp time_);
	
	kimgbo::net::EventLoop* m_loop;
	kimgbo::net::TcpServer m_server;
};

#endif