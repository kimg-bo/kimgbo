#include <stdio.h>
#include <set>
#include <functional>
#include <memory>
#include "Logging.h"
#include "Mutex.h"
#include "EventLoop.h"
#include "TcpServer.h"
#include "codec.h"
#include "ThreadLocalSingleton.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
class ChatServer
{
public:
  ChatServer(EventLoop* loop,
             const InetAddress& listenAddr)
  : m_loop(loop),
    m_server(m_loop, listenAddr, "ChatServer"),
    m_codec(std::bind(&ChatServer::onStringMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
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
  	m_server.setThreadInitCallback(std::bind(&ChatServer::threadInit, this, std::placeholders::_1));
    m_server.start();
  }

private:
	typedef std::set<TcpConnectionPtr> ConnectionList;
	
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
        << conn->peerAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");
    
    /*启动线程之前执行的初始化操作*/
    if (conn->connected())
    {
      m_connections.instance().insert(conn);
    }
    else
    {
      m_connections.instance().erase(conn);
    }
  }
  
  /*初始化线程之前*/
  void threadInit(EventLoop* loop)
  {
  	assert(m_connections.pointer() == NULL);
  	m_connections.instance();
  	assert(m_connections.pointer() != NULL);
  	
  	MutexLockGuard lock(m_mutex); //有可能同时创建的多个I/O线程会同时执行threadInit函数，所以次处需要加锁保护数据
  	m_loops.insert(loop); 
  }

	/*业务调度分发*/
  void onStringMessage(const TcpConnectionPtr&, const kimgbo::string& message, Timestamp)
  {
  	EventLoop::Functor f = std::bind(&ChatServer::distributeMessage, this, message);
    LOG_DEBUG;
    
    MutexLockGuard lock(m_mutex);
    for (std::set<EventLoop*>::iterator it = m_loops.begin(); it != m_loops.end(); ++it)
    {
      (*it)->queueInLoop(f); //queueInLoop将其转到EventLoop所属线程中执行,muduo中将线程通知机制抽象化了，通知调用线程所属的EventLoop直接转入
      											 //相关线程中执行
    }
    LOG_DEBUG;
  }
  
  /*各个线程具体的消息转发处理函数*/
  void distributeMessage(const kimgbo::string& message)
  {
  	LOG_DEBUG << "begin";
  	for (ConnectionList::iterator it = m_connections.instance().begin(); it != m_connections.instance().end(); ++it)
    {
      m_codec.send((*it).get(), message);
    }
    LOG_DEBUG << "end";
  }
  
private:
  EventLoop* m_loop;
  TcpServer m_server;
  LengthHeaderCodec m_codec;
  MutexLock m_mutex;
  ThreadLocalSingleton<ConnectionList> m_connections; //定义线程本地存储数据
  std::set<EventLoop*> m_loops; //存储各个线程的事件循环对象，方便调度
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