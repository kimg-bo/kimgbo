#ifndef _CHARGEN_H_
#define _CHARGEN_H_

#include "TcpServer.h"

class ChargenServer
{
public:
	ChargenServer(kimgbo::net::EventLoop* loop, const kimgbo::net::InetAddress& listenAddr, bool print = false);
		
	void start();
	
private:
	void OnConnection(const kimgbo::net::TcpConnectionPtr& conn);
	void OnMessage(const kimgbo::net::TcpConnectionPtr& conn, kimgbo::net::Buffer* buf, kimgbo::Timestamp time_);
	void OnWriteComplete(const kimgbo::net::TcpConnectionPtr& conn);
	
	void printThroughput();
	
private:
	kimgbo::net::EventLoop* m_loop;
	kimgbo::net::TcpServer m_server;
		
	kimgbo::string m_message;
	int64_t m_transferred;
	kimgbo::Timestamp m_startTime;
};

#endif