#include <functional>
#include <utility>
#include <stdio.h>
#include <unistd.h>
#include "EventLoop.h"
#include "TcpClient.h"
#include "InetAddress.h"
#include "Endian.h"
#include "Timestamp.h"
#include "Logging.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
class TimeClient
{
public:
	TimeClient(EventLoop* loop, const InetAddress& serverAddr):m_loop(loop), m_client(m_loop, serverAddr, "TimeClient")
	{
		m_client.setConnectionCallback(std::bind(&TimeClient::OnConnection, this, std::placeholders::_1));
		m_client.setMessageCallback(std::bind(&TimeClient::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
	
	void connect()
	{
		m_client.connect();
	}
	
private:
	void OnConnection(const TcpConnectionPtr& conn)
	{
		LOG_INFO << conn->localAddress().toIpPort() << " -> " << conn->peerAddress().toIpPort() 
						 << " is " << (conn->connected() ? "UP" : "DOWN");
		if(!conn->connected())
		{
			m_loop->quit();
		}
	}
	
	void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp recieveTime)
	{
		if(buf->readableBytes() >= sizeof(int32_t))
		{
			const void* data = buf->peek();
			int32_t be32 = *static_cast<const int32_t*>(data);
			buf->retrieve(sizeof(int32_t));
			time_t time_ = sockets::networkToHost32(be32);
			Timestamp ts(static_cast<int64_t>(time_)*Timestamp::kMicroSecondsPerSecond);
			LOG_INFO << "Server Time: " << time_ << " , " << ts.toFormattedString();
		}
		else
		{
			LOG_INFO << conn->name() << " no enough data " << buf->readableBytes() << " at " << recieveTime.toFormattedString();
		}
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
		InetAddress serverAddr(argv[1], 2009);
		TimeClient client(&loop, serverAddr);
		client.connect();
		loop.loop();
	}
	else
	{
		printf("Usage: %s host_ip\n", argv[0]);
	}
	
	return 0;
}