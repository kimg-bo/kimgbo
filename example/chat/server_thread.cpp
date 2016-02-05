#include <stdio.h>
#include <set>
#include <functional>
#include <memory>
#include "Logging.h"
#include "Mutex.h"
#include "EventLoop.h"
#include "TcpServer.h"
#include "codec.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
class ChatServer
{
public:
  ChatServer(EventLoop* loop,
             const InetAddress& listenAddr)
  : m_loop(loop),
    m_server(m_loop, listenAddr, "ChatServer"),
    m_codec(std::bind(&ChatServer::onStringMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)),
    m_connections(new ConnectionList)
  {
    m_server.setConnectionCallback(
        std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    m_server.setMessageCallback(
        std::bind(&LengthHeaderCodec::onMessage, &m_codec, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  }

  void setThreadNum(int numThreads)
  {
    m_server.setThreadNum(numThreads);
  }

  void start()
  {
    m_server.start();
  }

private:
	typedef std::set<TcpConnectionPtr> ConnectionList;
	typedef std::shared_ptr<ConnectionList> ConnectionListPtr;
	
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
        << conn->peerAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");

    MutexLockGuard lock(m_mutex);
    LOG_INFO << "lock(m_mutex) ok"; 
    if(!m_connections.unique())
    {
    	LOG_INFO << "m_connections.unique()."; 
    	m_connections.reset(new ConnectionList(*m_connections));
    }
    assert(m_connections.unique());
    
    if (conn->connected())
    {
    	LOG_INFO << "insert before."; 
      m_connections->insert(conn);
      LOG_INFO << "insert ok"; 
    }
    else
    {
      m_connections->erase(conn);
    }
  }
  
  ConnectionListPtr getConnectionList()
  {
  	MutexLockGuard lock(m_mutex);
  	return m_connections;
  }

  void onStringMessage(const TcpConnectionPtr&, const kimgbo::string& message, Timestamp)
  {
    ConnectionListPtr connections = getConnectionList();
    for (ConnectionList::iterator it = connections->begin(); it != connections->end(); ++it)
    {
      m_codec.send((*it).get(), message);
    }
  }
  
private:
  EventLoop* m_loop;
  TcpServer m_server;
  LengthHeaderCodec m_codec;
  MutexLock m_mutex;
  ConnectionListPtr m_connections;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  if (argc > 1)
  {
    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress serverAddr(port);
    ChatServer server(&loop, serverAddr);
    if (argc > 2)
    {
      server.setThreadNum(atoi(argv[2]));
    }
    server.start();
    loop.loop();
  }
  else
  {
    printf("Usage: %s port [thread_num]\n", argv[0]);
  }
}