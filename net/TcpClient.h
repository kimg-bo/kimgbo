#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <type_traits>
#include <tr1/memory>
//#include <memory>
#include <functional>
#include "Mutex.h"
#include "TcpConnection.h"

namespace kimgbo
{
	namespace net
	{
		class Connector;
		typedef std::tr1::shared_ptr<Connector> ConnectorPtr;

		class TcpClient
		{
 		public:
  		// TcpClient(EventLoop* loop);
  		// TcpClient(EventLoop* loop, const string& host, uint16_t port);
  		TcpClient(EventLoop* loop, const InetAddress& serverAddr, const string& name);
  		~TcpClient();  // force out-line dtor, for scoped_ptr members.

  		void connect();
  		void disconnect();
  		void stop();

  		TcpConnectionPtr connection() const
  		{
   			MutexLockGuard lock(m_mutex);
    		return m_connection;
  		}

  		bool retry() const;
  		void enableRetry() { m_retry = true; }

  		/// Set connection callback.
  		/// Not thread safe.
  		void setConnectionCallback(const ConnectionCallback& cb)
  		{ m_connectionCallback = cb; }

  		/// Set message callback.
  		/// Not thread safe.
  		void setMessageCallback(const MessageCallback& cb)
  		{ m_messageCallback = cb; }

  		/// Set write complete callback.
  		/// Not thread safe.
  		void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  		{ m_writeCompleteCallback = cb; }

 		private:
  		/// Not thread safe, but in loop
  		void newConnection(int sockfd);
  		/// Not thread safe, but in loop
  		void removeConnection(const TcpConnectionPtr& conn);

  		EventLoop* m_loop;
  		ConnectorPtr m_connector; // avoid revealing Connector
  		const string m_name;
  		ConnectionCallback m_connectionCallback;
  		MessageCallback m_messageCallback;
  		WriteCompleteCallback m_writeCompleteCallback;
  		bool m_retry;   // atmoic 用于标识当新建连接被断开后，是否继续重连
  		bool m_connect; // atomic
  		// always in loop thread
  		int m_nextConnId;
  		mutable MutexLock m_mutex;
  		TcpConnectionPtr m_connection; // @BuardedBy mutex_
		};
	}
}

#endif