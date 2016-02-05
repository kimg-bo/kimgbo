#include <stdio.h>
#include <iostream>
#include <functional>
#include "Logging.h"
#include "Mutex.h"
#include "EventLoopThread.h"
#include "TcpClient.h"
#include "codec.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
class ChatClient
{
public:
	ChatClient(EventLoop* loop, const InetAddress& serverAddr)
    : m_loop(loop),
      m_client(loop, serverAddr, "ChatClient"),
      m_codec(std::bind(&ChatClient::onStringMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
  {
    m_client.setConnectionCallback(std::bind(&ChatClient::onConnection, this, std::placeholders::_1));
    m_client.setMessageCallback(std::bind(&LengthHeaderCodec::onMessage, &m_codec, 
    																					std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_client.enableRetry();
  }
  
	void connect()
  {
    m_client.connect();
  }

  void disconnect()
  {
    // client_.disconnect();
  }
  
  void write(const StringPiece& message)
  {
    MutexLockGuard lock(m_mutex);
    if (m_connection)
    {
      m_codec.send(m_connection.get(), message);
    }
  }

private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    MutexLockGuard lock(m_mutex);
    if (conn->connected())
    {
      m_connection = conn;
    }
    else
    {
      m_connection.reset();
    }
  }

  void onStringMessage(const TcpConnectionPtr&, const kimgbo::string& message, Timestamp)
  {
    printf("<<< %s\n", message.c_str());
  }
  
private:
	EventLoop* m_loop;
  TcpClient m_client;
  LengthHeaderCodec m_codec;
  MutexLock m_mutex;
  TcpConnectionPtr m_connection;
};

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid();
  if (argc > 2)
  {
    EventLoopThread loopThread;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress serverAddr(argv[1], port);

    ChatClient client(loopThread.startLoop(), serverAddr);
    client.connect();
    std::string line;
    while (std::getline(std::cin, line))
    {
      client.write(line);
    }
    client.disconnect();
  }
  else
  {
    printf("Usage: %s host_ip port\n", argv[0]);
  }
	
	return 0;
}