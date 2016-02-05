#include <functional>
#include "Timestamp.h"
#include "echo.h"
#include "Logging.h"

EchoServer::EchoServer(kimgbo::net::EventLoop* loop, const kimgbo::net::InetAddress& listenAddr)
	:m_loop(loop),
		m_server(m_loop, listenAddr, "EchoServer")
{
	m_server.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
	m_server.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
	
void EchoServer::start()
{
	LOG_INFO << "EchoServer Start " ;
	m_server.start();
}
	
void EchoServer::onConnection(const kimgbo::net::TcpConnectionPtr& conn)
{
	LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
}
		
void EchoServer::onMessage(const kimgbo::net::TcpConnectionPtr& conn, kimgbo::net::Buffer* buf, kimgbo::Timestamp time_)
{
	kimgbo::string msg(buf->retrieveAllAsString());
	LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, " << "data received at " << time_.toString();;
	
	conn->send(msg);
}