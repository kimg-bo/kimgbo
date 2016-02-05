#include "EventLoop.h"
#include "echo.h"
#include "Logging.h"

int main(int argc, char* args[])
{
	LOG_INFO << "pid = " << getpid();
	kimgbo::net::EventLoop loop;
	kimgbo::net::InetAddress listenAddr(2007);
	EchoServer server(&loop, listenAddr);
	server.start();
	loop.loop();
	
	return 0;
}