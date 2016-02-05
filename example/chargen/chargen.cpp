#include <functional>
#include "chargen.h"
#include "EventLoop.h"
#include "Timestamp.h"
#include "Logging.h"

using namespace kimgbo;
using namespace kimgbo::net;

ChargenServer::ChargenServer(kimgbo::net::EventLoop* loop, const kimgbo::net::InetAddress& listenAddr, bool print)
	: m_loop(loop),
		m_server(m_loop, listenAddr, "ChargenServer"),
		m_transferred(0),
		m_startTime(Timestamp::now())
{
	m_server.setConnectionCallback(std::bind(&ChargenServer::OnConnection, this, std::placeholders::_1));
	m_server.setMessageCallback(std::bind(&ChargenServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_server.setWriteCompleteCallback(std::bind(&ChargenServer::OnWriteComplete, this, std::placeholders::_1));
		
	if(print)
	{
		m_loop->runEvery(3.0, std::bind(&ChargenServer::printThroughput, this));
	}
	
	kimgbo::string line;
	for (int i = 33; i < 127; ++i)
  {
    line.push_back(char(i));
  }
  line += line;

  for (size_t i = 0; i < 127-33; ++i)
  {
    m_message += line.substr(i, 72) + '\n';
  }
}

void ChargenServer::start()
{
	m_server.start();
}
	
void ChargenServer::OnConnection(const kimgbo::net::TcpConnectionPtr& conn)
{
	LOG_INFO << "ChargenServer - " << conn->peerAddress().toIpPort() << " -> " 
					 << conn->localAddress().toIpPort() << " is "
					 << (conn->connected() ? "UP" : "DOWN");
	if(conn->connected())
	{
		conn->setTcpNoDelay(true);
		conn->send(m_message);
	}
}
	
void ChargenServer::OnMessage(const kimgbo::net::TcpConnectionPtr& conn, kimgbo::net::Buffer* buf, kimgbo::Timestamp time_)
{
	kimgbo::string msg(buf->retrieveAllAsString());
	LOG_INFO << conn->name() << " discard " << msg.size() << " bytes recieved at " << time_.toString();
}

void ChargenServer::OnWriteComplete(const kimgbo::net::TcpConnectionPtr& conn)
{
	m_transferred += m_message.size();
	conn->send(m_message);
}

void ChargenServer::printThroughput()
{
	Timestamp endTime = Timestamp::now();
	double time_ = timeDifference(endTime, m_startTime);
	
	printf("time_: %f\n", time_);
	
	printf("%4.3f MiB/s \n", static_cast<double>(m_transferred)/time_/1024/1024);
	m_transferred = 0;
	m_startTime = endTime;
}