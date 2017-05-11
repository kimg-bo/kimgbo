#include <stdio.h>
#include <unistd.h>
#include <mcheck.h>
#include <utility>
#include <functional>
#include "TcpServer.h"
#include "EventLoop.h"
#include "Atomic.h"
#include "thread.h"
#include "threadpool.h"
#include "InetAddress.h"
#include "Logging.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
class LockfreeServer
{
public:
	LockfreeServer(EventLoop* loop, const InetAddress& listenAddr, int numThreads)
		: m_loop(loop),
			m_server(m_loop, listenAddr, "SudokuServer"),
			m_threadpool(true, 1000, 800, numThreads, numThreads, 10),
			m_numThreads(numThreads),
			m_startTime(Timestamp::now())
	{
		m_server.setConnectionCallback(std::bind(&LockfreeServer::OnConnection, this, std::placeholders::_1));
		m_server.setMessageCallback(std::bind(&LockfreeServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
	
	void start()
	{
		LOG_INFO << "starting " << m_numThreads << " threads.";
		m_threadpool.start();
		m_server.start();
	}
	
private:
	void OnConnection(const TcpConnectionPtr& conn)
	{
		LOG_TRACE << conn->peerAddress().toIpPort() << " -> "
							<< conn->localAddress().toIpPort() << " is "
							<< (conn->connected() ? "UP" : "DOWN");
	}
	
	void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time_)
	{
		LOG_DEBUG << conn->name();
		
		kimgbo::string msg(buf->retrieveAllAsString());
		m_threadpool.run(std::bind(&execute, conn, msg));
	}
	
	static void execute(const TcpConnectionPtr& conn, const kimgbo::string& msg)
	{
		LOG_DEBUG << conn->name();
		conn->send(msg);
	}
	
private:
	EventLoop* m_loop;
	TcpServer m_server;
	ThreadPool m_threadpool;
	int m_numThreads;
	Timestamp m_startTime;
};

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
	
	int numThreads = 0;
	
	if(argc > 1)
	{
		numThreads = atoi(argv[1]);
	}
	
	EventLoop loop;
	InetAddress listenAddr(2011);
	LockfreeServer server(&loop, listenAddr, numThreads);
	server.start();
	loop.loop();
	
	return 0;
}