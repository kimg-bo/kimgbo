#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <map>
#include <tr1/memory>
#include "Types.h"
#include "TcpConnection.h"

namespace kimgbo
{
	namespace net
	{
		class Acceptor;
		class EventLoop;
		class EventLoopThreadPool;
		
		class TcpServer
		{
		 public:
		 		typedef std::function<void(EventLoop*)> ThreadInitCallback;

  			//TcpServer(EventLoop* loop, const InetAddress& listenAddr);
  			TcpServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg);
  			~TcpServer();  // force out-line dtor, for scoped_ptr members.

  			const string& hostport() const { return m_hostport; }
  			const string& name() const { return m_name; }

  			/// Set the number of threads for handling input.
 			 	///
  			/// Always accepts new connection in loop's thread.
  			/// Must be called before @c start
  			/// @param numThreads
  			/// - 0 means all I/O in loop's thread, no thread will created.
  			///   this is the default value.
  			/// - 1 means all I/O in another thread.
  			/// - N means a thread pool with N threads, new connections
  			///   are assigned on a round-robin basis.
  			void setThreadNum(int numThreads);
  			void setThreadInitCallback(const ThreadInitCallback& cb)
  			{ m_threadInitCallback = cb; }

  			/// Starts the server if it's not listenning.
  			///
  			/// It's harmless to call it multiple times.
  			/// Thread safe.
  			void start();

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
  			void newConnection(int sockfd, const InetAddress& peerAddr);
  			/// Thread safe.
  			void removeConnection(const TcpConnectionPtr& conn);
  			/// Not thread safe, but in loop
  			void removeConnectionInLoop(const TcpConnectionPtr& conn);

  			typedef std::map<string, TcpConnectionPtr> ConnectionMap;

  			EventLoop* m_loop;  // the acceptor loop
  			const string m_hostport;
  			const string m_name;
  			std::unique_ptr<Acceptor> m_acceptor; // avoid revealing Acceptor
  			std::unique_ptr<EventLoopThreadPool> m_threadPool;
  			ConnectionCallback m_connectionCallback;
  			MessageCallback m_messageCallback;
  			WriteCompleteCallback m_writeCompleteCallback;
  			ThreadInitCallback m_threadInitCallback;
  			bool m_started;
  			// always in loop thread
  			int m_nextConnId;
  			ConnectionMap m_connections;
		};
	}
}

#endif