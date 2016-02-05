#include <functional>
#include "echo.h"
#include "Logging.h"

EchoServer::EchoServer(kimgbo::net::EventLoop* loop, const kimgbo::net::InetAddress& listenAddr, int maxConnections)
	: m_loop(loop),
    m_server(loop, listenAddr, "EchoServer"),
    m_numConnected(0),
    kMaxConnections(maxConnections)
{
	m_server.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
  m_server.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void EchoServer::start()
{
	m_server.start();
}
	
void EchoServer::onConnection(const kimgbo::net::TcpConnectionPtr& conn)
{
	LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
           	
	if(conn->connected())
	{
		++m_numConnected;
		if (m_numConnected > kMaxConnections)
    {
    	conn->send("too many connect.\r\n", strlen("too many connect.\r\n"));
      conn->shutdown();
    }
	}
	else
	{
		--m_numConnected;
	}
	LOG_INFO << "numConnected = " << m_numConnected;
}

void EchoServer::onMessage(const kimgbo::net::TcpConnectionPtr& conn, kimgbo::net::Buffer* buf, kimgbo::Timestamp time_)
{
	kimgbo::string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " echo " << msg.size() << " bytes at " << time_.toString();
  conn->send(msg);
}