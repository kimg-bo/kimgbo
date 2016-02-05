#include "echo.h"
#include <stdio.h>
#include "Logging.h"
#include "EventLoop.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
int main(int argc, char* argv[])
{
	EventLoop loop;
  InetAddress listenAddr(2007);
  int idleSeconds = 10;
  if (argc > 1)
  {
    idleSeconds = atoi(argv[1]);
  }
  LOG_INFO << "pid = " << getpid() << ", idle seconds = " << idleSeconds;
  EchoServer server(&loop, listenAddr, idleSeconds);
  server.start();
  loop.loop();
	
	return 0;
}