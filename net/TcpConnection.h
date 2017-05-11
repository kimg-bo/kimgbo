#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <iostream>
#include <functional>
#include <type_traits>
#include <tr1/memory>
//#include <memory>
#include "Mutex.h"
#include "StringPiece.h"
#include "Types.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "InetAddress.h"
#include "Socket.h"
#include "EventLoop.h"
#include "anyone.h"

namespace kimgbo
{
	namespace net
	{
		class Channel;
		
		class TcpConnection :public std::tr1::enable_shared_from_this<TcpConnection>
		{
		public:
  		/// Constructs a TcpConnection with a connected sockfd
  		///
  		/// User should not create this object.
  		TcpConnection(EventLoop* loop, const string& name, int sockfd, const InetAddress& localAddr, 
  										const InetAddress& peerAddr);
  		~TcpConnection();

  		EventLoop* getLoop() const { return m_loop; }
  		const string& name() const { return m_name; }
  		const InetAddress& localAddress() { return m_localAddr; }
  		const InetAddress& peerAddress() { return m_peerAddr; }
  		bool connected() const { return m_state == kConnected; }

 		 	// void send(string&& message); // C++11
  		void send(const void* message, size_t len);
  		void send(const StringPiece& message);
  		// void send(Buffer&& message); // C++11
  		void send(Buffer* message);  // this one will swap data
  		void shutdown(); // NOT thread safe, no simultaneous calling
  		void setTcpNoDelay(bool on);

  		void setContext(const any& context)
  		{ m_context = context; }

  		const any& getContext() const
  		{ return m_context; }

  		any* getMutableContext()
  		{ return &m_context; }

  		void setConnectionCallback(const ConnectionCallback& cb)
  		{ m_connectionCallback = cb; }

  		void setMessageCallback(const MessageCallback& cb)
  		{ m_messageCallback = cb; }

  		void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  		{ m_writeCompleteCallback = cb; }

  		void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
  		{ m_highWaterMarkCallback = cb; m_highWaterMark = highWaterMark; }

  		Buffer* inputBuffer()
  		{ return &m_inputBuffer; }

  		/// Internal use only.
  		void setCloseCallback(const CloseCallback& cb)
  		{ m_closeCallback = cb; }

  		// called when TcpServer accepts a new connection
  		void connectEstablished();   // should be called only once
  		// called when TcpServer has removed me from its map
  		void connectDestroyed();  // should be called only once
		private:
			enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
			void handleRead(Timestamp receiveTime);
			void handleWrite();
			void handleClose();
			void handleError();
			
			/*
			void sendInLoop(const StringPiece& message);
			void sendInLoop(const void* message, size_t len);
			*/
			
			void sendInLoopStringPiece(const StringPiece& message);
			void sendInLoopMessage(const void* message, size_t len);
			
			void shutdownInLoop();
			void setState(StateE s) { m_state = s; }
			
			EventLoop* m_loop;
			string m_name;
			StateE m_state;
			
			std::unique_ptr<Socket> m_socket; //TcpConnection对象销毁时会同时调用Socket的析构函数进而关闭fd
			std::unique_ptr<Channel> m_channel;
  		InetAddress m_localAddr;
  		InetAddress m_peerAddr;
  		ConnectionCallback m_connectionCallback;
  		MessageCallback m_messageCallback;
  		WriteCompleteCallback m_writeCompleteCallback;
  		HighWaterMarkCallback m_highWaterMarkCallback;
  		CloseCallback m_closeCallback;
  		size_t m_highWaterMark;
  		Buffer m_inputBuffer;
  		Buffer m_outputBuffer; // FIXME: use list<Buffer> as output buffer.
  		
  		any m_context;
		};
		typedef std::tr1::shared_ptr<TcpConnection> TcpConnectionPtr;
	}
}

#endif