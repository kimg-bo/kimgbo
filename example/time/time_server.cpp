#include "kimg_time.h"
#include "EventLoop.h"
#include "Logging.h"

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid();
	EventLoop loop;
	InetAddress listenAddr(2009);
	TimeServer server(&loop, listenAddr);
	server.start();
	loop.loop();
	
	return 0;
}