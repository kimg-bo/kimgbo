#include <assert.h>
#include <stdio.h>
#include <functional>
#include <vector>
#include <memory>
#include "Atomic.h"
#include "Logging.h"
#include "Mutex.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "TcpClient.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
AtomicInt32 g_aliveConnections;
AtomicInt32 g_disaliveConnections;
int g_connections = 4;
EventLoop* g_loop;
	
class FileClient
{
public:
	FileClient(EventLoop* loop, const InetAddress& serverAddr, const kimgbo::string& id)
		: m_loop(loop),
			m_client(m_loop, serverAddr, "FileClient")
	{
		m_client.setConnectionCallback(std::bind(&FileClient::OnConnection, this, std::placeholders::_1));
		m_client.setMessageCallback(std::bind(&FileClient::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		kimgbo::string filename = "RecvFileClient" + id;
		m_fp = ::fopen(filename.c_str(), "we");
		assert(m_fp);
	}
	
	~FileClient()
	{
		::fclose(m_fp);
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
             	
    if(conn->connected())
    {
    	m_connection = conn;
    	if(g_aliveConnections.incrementAndGet() == g_connections)
    	{
    		LOG_INFO << "all connected";
    	}
    	else
    	{
    		m_connection.reset();
    		
    		if(g_aliveConnections.incrementAndGet() == g_connections)
    		{
    			LOG_INFO << "all disconnected";
    			g_loop->quit();
    		}
    	}
    }
	}
	
	void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time_)
	{
		fwrite(buf->peek(), 1, buf->readableBytes(), m_fp);
		buf->retrieveAll();
	}
	
private:
	EventLoop* m_loop;
  TcpClient m_client;
  TcpConnectionPtr m_connection;
  FILE* m_fp;
};

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid();
	
	if(argc > 1)
	{
		int port = atoi(argv[1]);
		EventLoop loop;
  	g_loop = &loop;
  
  	EventLoopThreadPool loopPool(g_loop);
  	loopPool.setThreadNum(2);
  	loopPool.start();
  
  	std::vector<std::shared_ptr<FileClient>> clients;
  	InetAddress serverAddr("115.29.37.112", port);
  
  	for (int i = 0; i < g_connections; ++i)
  	{
  		char buf[8];
  		snprintf(buf, sizeof(buf), "%d", i+1);
  		std::shared_ptr<FileClient> client(new FileClient(loopPool.getNextLoop(), serverAddr, buf));
  		clients.push_back(client);
  		clients[i]->connect();
  		usleep(200);
  	}
  
  	loop.loop();
		usleep(2000);
	}
	else
	{
		fprintf(stderr, "Usage: %s server_port\n", argv[0]);
	}
  
	return 0;
}