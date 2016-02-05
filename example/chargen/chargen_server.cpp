#include "chargen.h"
#include "EventLoop.h"
#include "Logging.h"

using namespace kimgbo;
using namespace kimgbo::net;

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid();
	EventLoop loop;
	InetAddress listenAddr(2010);
	ChargenServer server(&loop, listenAddr, true);
	server.start();
	loop.loop();
	
	return 0;
}