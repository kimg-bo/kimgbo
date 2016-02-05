#include <stdio.h>
#include <unistd.h>
#include <mcheck.h>
#include <functional>
#include <utility>
#include "sudoku.h"
#include "Atomic.h"
#include "thread.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpServer.h"
#include "Logging.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
class SudokuServer
{
public:
	SudokuServer(EventLoop* loop, const InetAddress& listenAddr, int numThreads)
		: m_loop(loop),
			m_server(m_loop, listenAddr, "SudokuServer"),
			m_startTime(Timestamp::now())
	{
		m_server.setConnectionCallback(std::bind(&SudokuServer::OnConnection, this, std::placeholders::_1));
		m_server.setMessageCallback(std::bind(&SudokuServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		m_server.setThreadNum(numThreads);
	}
	
	void start()
	{
		LOG_INFO << "starting " << m_numThreads << " threads.";
		m_server.start();
	}
	
private:
	void OnConnection(const TcpConnectionPtr& conn)
	{
		LOG_TRACE << conn->peerAddress().toIpPort() << " -> "
        			<< conn->localAddress().toIpPort() << " is "
        			<< (conn->connected() ? "UP" : "DOWN");
	}
	
	void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time_)
	{
		LOG_DEBUG << conn->name();
    size_t len = buf->readableBytes();
    while (len >= kCells + 2)
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        kimgbo::string request(buf->peek(), crlf);
        buf->retrieveUntil(crlf + 2);
        len = buf->readableBytes();
        if (!processRequest(conn, request))
        {
          conn->send("Bad Request!\r\n");
          conn->shutdown();
          break;
        }
      }
      else if (len > 100) // id + ":" + kCells + "\r\n"
      {
        conn->send("Id too long!\r\n");
        conn->shutdown();
        break;
      }
      else
      {
        break;
      }
    }
	}
	
	bool processRequest(const TcpConnectionPtr& conn, const kimgbo::string& request)
  {
    kimgbo::string id;
    kimgbo::string puzzle;
    bool goodRequest = true;

    kimgbo::string::const_iterator colon = find(request.begin(), request.end(), ':');
    if (colon != request.end())
    {
      id.assign(request.begin(), colon);
      puzzle.assign(colon+1, request.end());
    }
    else
    {
      puzzle = request;
    }

    if (puzzle.size() == implicit_cast<size_t>(kCells))
    {
      LOG_DEBUG << conn->name();
      kimgbo::string result = solveSudoku(puzzle);
      if (id.empty())
      {
        conn->send(result+"\r\n");
      }
      else
      {
        conn->send(id+":"+result+"\r\n");
      }
    }
    else
    {
      goodRequest = false;
    }
    return goodRequest;
  }
	
private:
	EventLoop* m_loop;
	TcpServer m_server;
	int m_numThreads;
	Timestamp m_startTime;
};

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
		
	int numThreads = 0;
	
	if(argc > 1)
	{
		numThreads = atoi(argv[1]);
	}
	
	EventLoop loop;
	InetAddress listenAddr(2012);
	SudokuServer server(&loop, listenAddr, numThreads);
	server.start();
	loop.loop();
	
	return 0;
}