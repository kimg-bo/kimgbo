#include <functional>
#include "daytime.h"
#include "Logging.h"

DaytimeServer::DaytimeServer(kimgbo::net::EventLoop* loop, const kimgbo::net::InetAddress& listenAddr)
	: m_loop(loop),
		m_server(m_loop, listenAddr, "DayTimeServer")
{
	m_server.setConnectionCallback(std::bind(&DaytimeServer::OnConnection, this, std::placeholders::_1));
	m_server.setMessageCallback(std::bind(&DaytimeServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
	
void DaytimeServer::start()
{
	m_server.start();
}
	
void DaytimeServer::OnConnection(const kimgbo::net::TcpConnectionPtr& conn)
{
	LOG_INFO << "DaytimeServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
  if(conn->connected())
  {
  	conn->send(kimgbo::Timestamp::now().toFormattedString() + "\n");
  	conn->shutdown();
  }
}

void DaytimeServer::OnMessage(const kimgbo::net::TcpConnectionPtr& conn, kimgbo::net::Buffer* buf, kimgbo::Timestamp time_)
{
	kimgbo::string msg(buf->retrieveAllAsString());
	LOG_INFO << conn->name() << " discards " << msg.size() << " bytes received at " << time_.toString();
}