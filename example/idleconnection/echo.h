#ifndef IDLECONNECTION_ECHO_H
#define IDLECONNECTION_ECHO_H

#include <tr1/memory>
#include <vector>
#include "TcpServer.h"

class EchoServer
{
public:
	EchoServer(kimgbo::net::EventLoop* loop, const kimgbo::net::InetAddress& listenAddr, int idleSeconds);
  void start();

private:
	void onConnection(const kimgbo::net::TcpConnectionPtr& conn);
  void onMessage(const kimgbo::net::TcpConnectionPtr& conn, kimgbo::net::Buffer* buf, kimgbo::Timestamp time);
  void onTimer();
  void dumpConnectionBuckets() const;
  
private:
	typedef std::tr1::weak_ptr<kimgbo::net::TcpConnection> WeakTcpConnectionPtr;
	
	struct Entry
	{
		explicit Entry(const WeakTcpConnectionPtr& weakConn)
			: m_weakConn(weakConn)
		{
		}
		
		~Entry()
		{
			kimgbo::net::TcpConnectionPtr conn = m_weakConn.lock();
			if(conn)
			{
				conn->shutdown();
			}
		}
		
		WeakTcpConnectionPtr m_weakConn;
	};
	
private:
	typedef std::tr1::shared_ptr<Entry> EntryPtr;
	typedef std::tr1::weak_ptr<Entry> WeakEntryPtr;
	typedef std::vector<EntryPtr> Bucket;
	typedef std::vector<Bucket> WeakConnectionList;
		
	kimgbo::net::EventLoop* m_loop;
  kimgbo::net::TcpServer m_server;
  WeakConnectionList m_connectionBuckets;
  int m_idleSeconds;
  int m_nowIndex;
};

#endif //IDLECONNECTION_ECHO_H