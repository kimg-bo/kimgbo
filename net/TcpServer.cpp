#include <stdio.h>  // snprintf
#include <functional>
#include <tr1/memory>
#include "TcpServer.h"
#include "Logging.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"
#include "SocketOps.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg)
	  :m_loop(CHECK_NOTNULL(loop)),
    m_hostport(listenAddr.toIpPort()),
    m_name(nameArg),
    m_acceptor(new Acceptor(loop, listenAddr)),
    m_threadPool(new EventLoopThreadPool(loop)),
    m_connectionCallback(defaultConnectionCallback),
    m_messageCallback(defaultMessageCallback),
    m_started(false),
    m_nextConnId(1)
{
	m_acceptor->setNewConnectionCallback(std::bind(&TcpServer::newConnection, 
																					this, std::placeholders::_1, std::placeholders::_2));
}
  			
TcpServer::~TcpServer()
{
	m_loop->assertInLoopThread();
	LOG_TRACE << "TcpServer::~TcpServer [" << m_name << "] destructing";

  for (ConnectionMap::iterator it(m_connections.begin());
      it != m_connections.end(); ++it)
  {
    TcpConnectionPtr conn = it->second;
    it->second.reset();
    conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    conn.reset();
  }
}
  		
void TcpServer::setThreadNum(int numThreads)
{
	assert(0 <= numThreads);
  m_threadPool->setThreadNum(numThreads);
}
  			
void TcpServer::start()
{
	if (!m_started)
  {
    m_started = true;
    m_threadPool->start(m_threadInitCallback);
  }

  if (!m_acceptor->listenning())
  {
    m_loop->runInLoop(std::bind(&Acceptor::listen, m_acceptor.get()));
  }
}
  			
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
	m_loop->assertInLoopThread();
  EventLoop* ioLoop = m_threadPool->getNextLoop();
  char buf[32];
  snprintf(buf, sizeof(buf), ":%s#%d", m_hostport.c_str(), m_nextConnId);
  ++m_nextConnId;
  string connName = m_name + buf;

  LOG_INFO << "TcpServer::newConnection [" << m_name
           << "] - new connection [" << connName
           << "] from " << peerAddr.toIpPort();
           
  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
  m_connections[connName] = conn;
  conn->setConnectionCallback(m_connectionCallback);
  conn->setMessageCallback(m_messageCallback);
  conn->setWriteCompleteCallback(m_writeCompleteCallback);
  conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1)); // FIXME: unsafe
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
	m_loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
	m_loop->assertInLoopThread();
	LOG_INFO << "TcpServer::removeConnectionInLoop [" << m_name << "] - connection " << conn->name();
  size_t n = m_connections.erase(conn->name());
  (void)n;
  assert(n == 1);
  EventLoop* ioLoop = conn->getLoop();
  ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}