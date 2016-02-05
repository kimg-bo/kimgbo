#include "EventLoop.h"
#include "daytime.h"
#include "Logging.h"

using namespace kimgbo;
using namespace kimgbo::net;

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid();
	EventLoop loop;
	InetAddress listenAddr(2008);
	
	DaytimeServer server(&loop, listenAddr);
	server.start();
	loop.loop();
	
	return 0;
}