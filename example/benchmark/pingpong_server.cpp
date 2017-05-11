#include "TcpServer.h"

#include "Atomic.h"
#include "Logging.h"
#include "thread.h"
#include "EventLoop.h"
#include "InetAddress.h"

#include <boost/bind.hpp>
#include <functional>

#include <utility>

#include <mcheck.h>
#include <stdio.h>
#include <unistd.h>

using namespace kimgbo;
using namespace kimgbo::net;

void onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    conn->setTcpNoDelay(true);
  }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
{
  conn->send(buf);
}

int main(int argc, char* argv[])
{
  if (argc < 4)
  {
    fprintf(stderr, "Usage: server <address> <port> <threads>\n");
  }
  else
  {
    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
    Logger::setLogLevel(Logger::WARN);

    const char* ip = argv[1];
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress listenAddr(ip, port);
    int threadCount = atoi(argv[3]);

    EventLoop loop;

    TcpServer server(&loop, listenAddr, "PingPong");

    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);

    if (threadCount > 1)
    {
      server.setThreadNum(threadCount);
    }

    server.start();

		LOG_INFO << "start loop....";
    loop.loop();
  }
}

