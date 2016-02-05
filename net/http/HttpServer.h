#ifndef NET_HTTP_HTTPSERVER_H
#define NET_HTTP_HTTPSERVER_H

#include <functional>
#include "TcpServer.h"

namespace kimgbo
{
namespace net
{

class HttpRequest;
class HttpResponse;

/// A simple embeddable HTTP server designed for report status of a program.
/// It is not a fully HTTP 1.1 compliant server, but provides minimum features
/// that can communicate with HttpClient and Web browser.
/// It is synchronous, just like Java Servlet.
class HttpServer
{
 public:
  typedef std::function<void (const HttpRequest&, HttpResponse*)> HttpCallback;

  HttpServer(EventLoop* loop, const InetAddress& listenAddr, const string& name);

  ~HttpServer();  // force out-line dtor, for scoped_ptr members.

  /// Not thread safe, callback be registered before calling start().
  void setHttpCallback(const HttpCallback& cb)
  {
    m_httpCallback = cb;
  }

  void setThreadNum(int numThreads)
  {
    m_server.setThreadNum(numThreads);
  }

  void start();

 private:
  void onConnection(const TcpConnectionPtr& conn);
  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime);
  void onRequest(const TcpConnectionPtr&, const HttpRequest&);

  TcpServer m_server;
  HttpCallback m_httpCallback;
};

}
}

#endif  // MUDUO_NET_HTTP_HTTPSERVER_H
