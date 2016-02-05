#include "echo.h"
#include "Logging.h"
#include "EventLoop.h"

using namespace kimgbo;
using namespace kimgbo::net;

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  EventLoop loop;
  InetAddress listenAddr(2015);
  int maxConnections = 2;
  if (argc > 1)
  {
    maxConnections = atoi(argv[1]);
  }
  LOG_INFO << "maxConnections = " << maxConnections;
  EchoServer server(&loop, listenAddr, maxConnections);
  server.start();
  loop.loop();
}