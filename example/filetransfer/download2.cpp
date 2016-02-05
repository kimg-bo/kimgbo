#include <stdio.h>
#include <memory>
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
typedef std::shared_ptr<FILE> FilePtr;
	
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
			FilePtr ctx(fp, ::fclose);
			conn->setContext(ctx);
			char buf[kBufferSize];
			size_t nread = ::fread(buf, 1, sizeof(buf), fp);
			conn->send(buf, nread);
		}
		else
		{
			conn->shutdown();
			LOG_INFO << "FileServer - no such file";
		}
	}
}

void OnWriteComplete(const TcpConnectionPtr& conn)
{
	LOG_INFO << "FileServer - in onWriteComplete";
	
	const FilePtr fp = any_cast<const FilePtr>(conn->getContext());
	
	LOG_INFO << "FileServer - conn->getContext().cast<FilePtr>()";
	
	char buf[kBufferSize];
	
	printf("%d, %p\n", sizeof(fp.get()), fp.get());
	
	size_t nread = ::fread(buf, 1, sizeof(buf), fp.get());
		
		LOG_INFO << "::fread(buf, 1, sizeof(buf), fp.get())";
	if(nread > 0)
	{
		conn->send(buf, nread);
	}
	else
	{
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
		InetAddress listenAddr(2013);
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