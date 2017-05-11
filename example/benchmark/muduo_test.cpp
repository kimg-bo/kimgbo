#include <stdio.h>
#include <unistd.h>
#include <mcheck.h>
#include <utility>
#include <functional>
#include <time.h>
#include <string.h>
#include <string>
#include "TcpServer.h"
#include "EventLoop.h"
#include "Atomic.h"
#include "thread.h"
#include "threadpool.h"
#include "MutexThreadPool.h"
#include "InetAddress.h"
#include "Logging.h"
#include "Atomic.h"
#include "CircularBuffer.h"

using namespace kimgbo;
using namespace kimgbo::net;
using namespace kimgbo::detail;

static void solve(const int puzzle)
{
	int i=0;
	i++;
	i--;
}

/* 线程池性能测试 */
int main01(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
	int numThreads = 0;
	
	if(argc > 1)
	{
		numThreads = atoi(argv[1]);
	}
	
	EventLoop loop;
	ThreadPool m_threadpool(numThreads);
	m_threadpool.start();
	//MutexThreadPool m_threadpool;
	//m_threadpool.start(numThreads);
	
	Timestamp start(Timestamp::now());
	for(int i=0; i<100000; i++)
	{
		m_threadpool.run(std::bind(&solve, i));	
	}
	m_threadpool.stop();
	
	Timestamp end(Timestamp::now());			
	int delay = end.microSecondsSinceEpoch() - start.microSecondsSinceEpoch();
	LOG_INFO << delay;
	
	return 0;
}

int main02(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
	
	detail::AtomicIntegerT<unsigned int> index;
	index.getAndAdd(4294967294);
	unsigned int bb = 8;
	
	Timestamp start0(Timestamp::now());
	int mod = index.getAndAdd(1) % bb;
	Timestamp end0(Timestamp::now());
	int delay0 = end0.microSecondsSinceEpoch() - start0.microSecondsSinceEpoch();
	
	Timestamp start1(Timestamp::now());
	int a = 4294967291;
	int b = 7;
	mod = a % b;
	Timestamp end1(Timestamp::now());
	int delay1 = end1.microSecondsSinceEpoch() - start1.microSecondsSinceEpoch();
	
	Timestamp start2(Timestamp::now());
	char* ptr = "8";
	mod = atoi(ptr) % 7;
	Timestamp end2(Timestamp::now());
	int delay2 = end2.microSecondsSinceEpoch() - start2.microSecondsSinceEpoch();
	
	LOG_INFO << "Atomic: " << delay0 << ", num: " << delay1 << ", str: " << delay2;
	
	detail::AtomicIntegerT<unsigned int> index_;
	LOG_INFO << index_.getAndAdd(4294967294);
	LOG_INFO << index_.getAndAdd(1);
	LOG_INFO << index_.getAndAdd(1);
	LOG_INFO << index_.getAndAdd(1);
	LOG_INFO << index_.getAndAdd(1);
	
	return 0;
}

int main03(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
	int numThreads = 0;
	
	if(argc > 1)
	{
		numThreads = atoi(argv[1]);
	}
	
	EventLoop loop;
	ThreadPool m_threadpool(true, 180000, 110000, 1,numThreads, 0);
	m_threadpool.start();
	
	Timestamp start(Timestamp::now());
	for(int i=0; i<100000; i++)
	{
		m_threadpool.run(std::bind(&solve, i));
	}
	m_threadpool.stop();
	
	Timestamp end(Timestamp::now());			
	int delay = end.microSecondsSinceEpoch() - start.microSecondsSinceEpoch();
	LOG_INFO << delay;
	
	return 0;
}

/* CircularBuffer正常测试 */
int main04(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
	int dataSize = 0;//输入500
	
	if(argc > 1)
	{
		dataSize = atoi(argv[1]);
	}
	
	char* data = new char[dataSize];
	for(int i=0; i<dataSize; i++)
	{
		data[i] = 'a';
	}
	
	CircularBuffer buffer;
	buffer.append(data, dataSize);
	LOG_INFO << "readable: " << buffer.readableBytes() << ", writeable: " << buffer.writableBytes();
	
	kimgbo::string str = buffer.retrieveAsString(100);
	LOG_INFO << "read: " << str;
	LOG_INFO << "readable: " << buffer.readableBytes() << ", writeable: " << buffer.writableBytes();
	
	buffer.append(data, 500);
	LOG_INFO << "readable: " << buffer.readableBytes() << ", writeable: " << buffer.writableBytes();
	
	str = buffer.retrieveAsString(500);
	LOG_INFO << "read: " << str;
	LOG_INFO << "readable: " << buffer.readableBytes() << ", writeable: " << buffer.writableBytes();
	
	buffer.append(data, 100);
	LOG_INFO << "readable: " << buffer.readableBytes() << ", writeable: " << buffer.writableBytes();
	
	str = buffer.retrieveAllAsString();
	LOG_INFO << "read: " << str;
	LOG_INFO << "readable: " << buffer.readableBytes() << ", writeable: " << buffer.writableBytes();
	
	return 0;
}

/* CircularBuffer makeSpace测试 */
int main05(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
	int dataSize = 0;//输入1024
	
	if(argc > 1)
	{
		dataSize = atoi(argv[1]);
	}
	
	char* data = new char[dataSize];
	for(int i=0; i<dataSize; i++)
	{
		data[i] = 'a';
	}
	LOG_INFO << "data: " << data;
	
	CircularBuffer buffer;
	LOG_INFO << "append: " << dataSize;
	buffer.append(data, dataSize);
	LOG_INFO << "readable: " << buffer.readableBytes() << ", writeable: " << buffer.writableBytes();
	
	LOG_INFO << "read: " << 100;
	kimgbo::string str = buffer.retrieveAsString(100);
	LOG_INFO << "read: " << str;
	LOG_INFO << "readable: " << buffer.readableBytes() << ", writeable: " << buffer.writableBytes();
	LOG_INFO << "str Size: " << str.length();
	
	LOG_INFO << "append: " << 500;
	buffer.append(data, 500);
	LOG_INFO << "readable: " << buffer.readableBytes() << ", writeable: " << buffer.writableBytes();
	
	LOG_INFO << "read: " << 800;
	str = buffer.retrieveAsString(800);
	LOG_INFO << "read: " << str;
	LOG_INFO << "readable: " << buffer.readableBytes() << ", writeable: " << buffer.writableBytes();
	LOG_INFO << "str Size: " << str.length();
	
	LOG_INFO << "append: " << 1000;
	buffer.append(data, 1000);
	LOG_INFO << "append: " << 402;
	buffer.append(data, 402);
	LOG_INFO << "readable: " << buffer.readableBytes() << ", writeable: " << buffer.writableBytes();
	
	LOG_INFO << "append: " << 24;
	buffer.append(data, 24);
	LOG_INFO << "readable: " << buffer.readableBytes() << ", writeable: " << buffer.writableBytes();
	
	LOG_INFO << buffer.retrieveAsString(5000);
	str = buffer.retrieveAllAsString();
	
	std::cout << "str 5000: " << str;
	
	LOG_INFO << "read: " << str;
	LOG_INFO << "readable: " << buffer.readableBytes() << ", writeable: " << buffer.writableBytes();
	
	return 0;
}

/* CircularBuffer 与 Buffer 性能测试 */
int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
	int dataSize = 0;
	int rwSize = 0;
	
	if(argc > 1)
	{
		dataSize = atoi(argv[1]);
		rwSize = dataSize / 4;
	}
	
	char* data = new char[dataSize];
	for(int i=0; i<dataSize; i++)
	{
		data[i] = 'a';
	}
	
	Buffer buffer;
	//CircularBuffer buffer;
	buffer.append(data, dataSize);
	kimgbo::string str = buffer.retrieveAsString(rwSize+1);
	
	Timestamp start(Timestamp::now());
	for(int i=0; i<10000; i++)
	{
		buffer.append(data, rwSize);
		str = buffer.retrieveAsString(rwSize);
	}
	Timestamp end(Timestamp::now());			
	int delay = end.microSecondsSinceEpoch() - start.microSecondsSinceEpoch();
	LOG_INFO << delay;

	return 0;
}