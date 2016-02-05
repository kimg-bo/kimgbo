#ifndef __DAYTIME_H_
#define __DAYTIME_H_

#include "TcpServer.h"
#include "EventLoop.h"

class DaytimeServer
{
public:
	DaytimeServer(kimgbo::net::EventLoop* loop, const kimgbo::net::InetAddress& listenAddr);
	
	void start();
	
private:
	void OnConnection(const kimgbo::net::TcpConnectionPtr& conn);
	
	/*Œ¥ π”√*/
	void OnMessage(const kimgbo::net::TcpConnectionPtr& conn, kimgbo::net::Buffer* buf, kimgbo::Timestamp time_);
	
private:
	kimgbo::net::EventLoop* m_loop;
	kimgbo::net::TcpServer m_server;
};

#endif