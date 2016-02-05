#include <stdio.h>
#include "TcpServer.h"
#include "EventLoop.h"
#include "Logging.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
void OnHighWaterMark(const TcpConnectionPtr& conn, size_t len)
{
	LOG_INFO << "OnHighWaterMark " << len;
}

const int kBufferSize = 64*1024;
const char* g_file = NULL;
	
void OnConnection(const TcpConnectionPtr& conn)
{
	LOG_INFO << "FileServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
           	
	if(conn->connected())
	{
		LOG_INFO << "FileServer - Sending file " << g_file << conn->peerAddress().toIpPort();
		conn->setHighWaterMarkCallback(OnHighWaterMark, kBufferSize+1);
		
		FILE* fp = ::fopen(g_file, "rb");
		if(fp)
		{
			conn->setContext(fp);
			char buf[kBufferSize];
			size_t nread = ::fread(buf, 1, sizeof(buf), fp);
			conn->send(buf, nread);
			LOG_INFO << "FileServer - Sending file ";
		}
		else
		{
			conn->shutdown();
			LOG_INFO << "FileServer - no such file";
		}
	}
	else
	{
		if(!conn->getContext().empty())
		{
			//FILE* fp = any_cast<FILE*>(conn->getContext());
			FILE* fp = conn->getContext().cast<FILE*>();
			if(fp)
			{
				::fclose(fp);
			}
		}
	}
}

void OnWriteComplete(const TcpConnectionPtr& conn)
{
	LOG_INFO << "FileServer - in  OnWriteComplete";
		LOG_INFO << "before getContext()";
	FILE* fp = conn->getContext().cast<FILE*>();
		LOG_INFO << "getContext()";
	char buf[kBufferSize];
		LOG_INFO << "before ::fread(buf, 1, sizeof(buf), fp)";
	size_t nread = ::fread(buf, 1, sizeof(buf), fp);
		LOG_INFO << "::fread(buf, 1, sizeof(buf), fp)";
	if(nread > 0)
	{
		conn->send(buf, nread);
		LOG_INFO << "FileServer - Sending file ";
	}
	else
	{
		LOG_INFO << "FileServer - nread = 0";
		::fclose(fp);
		fp = NULL;
		conn->setContext(fp);
		conn->shutdown();
		LOG_INFO << "FileServer - done.";
	}
}

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid();
	
	if(argc > 1)
	{
		g_file = argv[1];
		
		EventLoop loop;
		InetAddress listenAddr(2014);
		TcpServer server(&loop, listenAddr, "FileServer");
		server.setConnectionCallback(OnConnection);
		server.setWriteCompleteCallback(OnWriteComplete);
		server.start();
		loop.loop();
	}
	else
	{
		fprintf(stderr, "Usage: %s file_for_downloading\n", argv[0]);
	}
	
	return 0;
}