.SUFFIXES: .cpp .o

CC=g++

SRCS=../../base/Timestamp.cpp\
			../../base/log.cpp\
			../../base/Date.cpp\
			../../base/TimeZone.cpp\
			../../base/LogStream.cpp\
			../../base/Logging.cpp\
			../../base/LogFile.cpp\
			../../base/Condition.cpp\
			../../base/CountDownLatch.cpp\
			../../base/thread.cpp\
			../../base/threadpool.cpp\
			../../base/FileUtil.cpp\
			../../base/ProcessInfo.cpp\
			../../net/Timer.cpp\
			../../net/TimerQueue.cpp\
			../../net/SocketOps.cpp\
			../../net/InetAddress.cpp\
			../../net/Socket.cpp\
			../../net/Buffer.cpp\
			../../net/boilerplate.cpp\
			../../net/Channel.cpp\
			../../net/Poller.cpp\
			../../net/TcpConnection.cpp\
			../../net/Acceptor.cpp\
			../../net/TcpServer.cpp\
			../../net/Connector.cpp\
			../../net/TcpClient.cpp\
			../../net/EventLoopThread.cpp\
			../../net/EventLoopThreadPool.cpp\
			../../net/EventLoop.cpp\
			../../net/poller/PollPoller.cpp\
			../../net/poller/EPollPoller.cpp\
			../../net/poller/DefaultPoller.cpp\
			../../net/http/HttpResponse.cpp\
			../../net/http/HttpServer.cpp\
			../../net/inspect/ProcessInspector.cpp\
			../../net/inspect/Inspector.cpp\
			./echo.cpp\
			./main.cpp
			
OBJS=$(SRCS:.cpp=.o)
EXEC=echo_server

all: $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) -lpthread
	@echo '----------------ok------------------'
	
.cpp.o:
	$(CC) -std=c++0x -Wall -g -o $@ -c $< -I/usr/include -I/usr/include/c++/4.8.2/tr1/tuple -I../../base/ -I../../net/ -I../../net/poller/ -I../../net/http/ -I../../net/inspect/
	
clean:
	rm -f $(OBJS)
	rm -f core*