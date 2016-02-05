#include <functional>
#include "kimg_time.h"
#include "Endian.h"
#include "Logging.h"

TimeServer::TimeServer(kimgbo::net::EventLoop* loop, const kimgbo::net::InetAddress& listenAddr)
	: m_loop(loop),
		m_server(m_loop, listenAddr, "TimeServer")
{
	m_server.setConnectionCallback(std::bind(&TimeServer::OnConnection, this, std::placeholders::_1));
	m_server.setMessageCallback(std::bind(&TimeServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
		
void TimeServer::start()
{
	m_server.start();
}

void TimeServer::OnConnection(const kimgbo::net::TcpConnectionPtr& conn)
{
	LOG_INFO << "TimeServer - " << conn->peerAddress().toIpPort() << " -> " << conn->localAddress().toIpPort() << " is " << (conn->connected() ? "UP" : "DOWN");
	if(conn->connected())
	{
		time_t now = ::time(NULL);
		int32_t be32 = sockets::hostToNetwork32(static_cast<int32_t>(now));
		conn->send(&be32, sizeof(be32));
		conn->shutdown();
	}
}

void TimeServer::OnMessage(const kimgbo::net::TcpConnectionPtr& conn, kimgbo::net::Buffer* buf, kimgbo::Timestamp time_)
{
	kimgbo::string msg(buf->retrieveAllAsString());
	LOG_INFO << conn->name() << " discards " << msg.size() << " bytes received at " << time_.toString();
}