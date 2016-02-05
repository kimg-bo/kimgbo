#ifndef KIMG_TIME_H_
#define KIMG_TIME_H_

#include "TcpServer.h"
#include "EventLoop.h"

class TimeServer
{
public:
	TimeServer(kimgbo::net::EventLoop* loop, const kimgbo::net::InetAddress& listenAddr);
		
	void start();
	
private:
	void OnConnection(const kimgbo::net::TcpConnectionPtr& conn);
	void OnMessage(const kimgbo::net::TcpConnectionPtr& conn, kimgbo::net::Buffer* buf, kimgbo::Timestamp time_);
	
private:
	kimgbo::net::EventLoop* m_loop;
	kimgbo::net::TcpServer m_server;
};

#endif //KIMG_TIME_H_