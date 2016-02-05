#include <stdio.h>
#include <unistd.h>
#include <utility>
#include <functional>
#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpClient.h"
#include "Logging.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
class ChargenClient
{
public:
	ChargenClient(EventLoop* loop, const InetAddress& serverAddr)
		: m_loop(loop),
			m_client(m_loop, serverAddr, "ChargenClient")
	{
		m_client.setConnectionCallback(std::bind(&ChargenClient::OnConnection, this, std::placeholders::_1));
		m_client.setMessageCallback(std::bind(&ChargenClient::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
	
	void connect()
	{
		m_client.connect();
	}
	
private:
	void OnConnection(const TcpConnectionPtr& conn)
	{
		LOG_INFO << conn->localAddress().toIpPort() << " -> "
						 << conn->peerAddress().toIpPort() << " is "
						 << (conn->connected() ? "UP" : "DOWN");
		if(!conn->connected())
		{
			m_loop->quit();
		}
	}
	
	void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
	{
		buf->retrieveAll();
	}
	
private:
	EventLoop* m_loop;
	TcpClient m_client;
};

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid();
	if(argc > 1)
	{
		EventLoop loop;
		InetAddress serverAddr(argv[1], 2010);
		ChargenClient client(&loop, serverAddr);
		client.connect();
		loop.loop();
	}
	else
	{
		printf("Usage: %s host_ip\n", argv[0]);
	}
	
	return 0;
}
