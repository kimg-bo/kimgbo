#ifndef SIMPLE_ECHO_ECHO_H
#define SIMPLE_ECHO_ECHO_H

#include "TcpServer.h"

class EchoServer
{
public:
	EchoServer(kimgbo::net::EventLoop* loop, const kimgbo::net::InetAddress& listenAddr, int maxConnections);

  void start();
	
private:
	void onConnection(const kimgbo::net::TcpConnectionPtr& conn);

  void onMessage(const kimgbo::net::TcpConnectionPtr& conn, kimgbo::net::Buffer* buf, kimgbo::Timestamp time_);
	
private:
	kimgbo::net::EventLoop* m_loop;
  kimgbo::net::TcpServer m_server;
  int m_numConnected; // should be atomic_int
  const int kMaxConnections;
};

#endif //SIMPLE_ECHO_ECHO_H