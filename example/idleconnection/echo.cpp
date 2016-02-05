#include <assert.h>
#include <stdio.h>
#include <functional>
#include "echo.h"
#include "Logging.h"
#include "EventLoop.h"

EchoServer::EchoServer(kimgbo::net::EventLoop* loop, const kimgbo::net::InetAddress& listenAddr, int idleSeconds)
	: m_loop(loop),
    m_server(loop, listenAddr, "EchoServer"),
    m_connectionBuckets(idleSeconds),
    m_idleSeconds(idleSeconds),
    m_nowIndex(0)
{
	m_server.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
  m_server.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  loop->runEvery(1.0, std::bind(&EchoServer::onTimer, this));
  m_connectionBuckets.resize(idleSeconds);
  dumpConnectionBuckets();
}

void EchoServer::start()
{
	m_server.start();
}

void EchoServer::onConnection(const kimgbo::net::TcpConnectionPtr& conn)
{
	LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
           	
	if(conn->connected())
	{
		EntryPtr entry(new Entry(conn));
		m_connectionBuckets[m_nowIndex].push_back(entry);
		dumpConnectionBuckets();
		WeakEntryPtr weakEntry(entry);
		conn->setContext(weakEntry);
	}
	else
	{
		assert(!conn->getContext().empty());
		WeakEntryPtr weakEntry(any_cast<WeakEntryPtr>(conn->getContext()));
		LOG_DEBUG << "Entry use_count = " << weakEntry.use_count();
	}
}

void EchoServer::onMessage(const kimgbo::net::TcpConnectionPtr& conn, kimgbo::net::Buffer* buf, kimgbo::Timestamp time_)
{
	kimgbo::string msg(buf->retrieveAllAsString());
  LOG_INFO << conn->name() << " echo " << msg.size() << " bytes at " << time_.toString();
  conn->send(msg);
  
  assert(!conn->getContext().empty());
  WeakEntryPtr weakEntry(any_cast<WeakEntryPtr>(conn->getContext()));
  EntryPtr entry(weakEntry.lock());
  if(entry)
  {
  	m_connectionBuckets[m_nowIndex].push_back(entry);
  	dumpConnectionBuckets();
  }
}

void EchoServer::onTimer()
{
	++m_nowIndex;
	m_nowIndex %= m_idleSeconds;
	m_connectionBuckets[m_nowIndex].clear();
	m_connectionBuckets[m_nowIndex].reserve(0);
	//std::vector<EntryPtr>(m_connectionBuckets[m_nowIndex]).swap(m_connectionBuckets[m_nowIndex].clear());
	dumpConnectionBuckets();
}

void EchoServer::dumpConnectionBuckets() const
{
	LOG_INFO << "size = " << m_connectionBuckets.size();
  int idx = 0;
  
  for(WeakConnectionList::const_iterator bucketI = m_connectionBuckets.begin(); 
  		bucketI != m_connectionBuckets.end(); ++bucketI, ++idx)
  {
  	const Bucket& bucket = *bucketI;
  	printf("[%d] len = %zd : ", idx, bucket.size());
  	for(Bucket::const_iterator it = bucket.begin(); it != bucket.end(); ++it)
  	{
  		bool connectionDead = (*it)->m_weakConn.expired();
      printf("(%ld)%s, ", it->use_count(), connectionDead ? " DEAD" : "");
  	}
  	puts("");
  }
}