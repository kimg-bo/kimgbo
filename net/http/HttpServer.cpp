#include <functional>
#include "HttpServer.h"
#include "Logging.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "anyone.h"

using namespace kimgbo;
using namespace kimgbo::net;

namespace kimgbo
{
namespace net
{
namespace detail
{

// FIXME: move to HttpContext class
bool processRequestLine(const char* begin, const char* end, HttpContext* context)
{
  bool succeed = false;
  const char* start = begin;
  const char* space = std::find(start, end, ' ');
  HttpRequest& request = context->request();
  if (space != end && request.setMethod(start, space))
  {
    start = space+1;
    space = std::find(start, end, ' ');
    if (space != end)
    {
      request.setPath(start, space);
      start = space+1;
      succeed = end-start == 8 && std::equal(start, end-1, "HTTP/1.");
      if (succeed)
      {
        if (*(end-1) == '1')
        {
          request.setVersion(HttpRequest::kHttp11);
        }
        else if (*(end-1) == '0')
        {
          request.setVersion(HttpRequest::kHttp10);
        }
        else
        {
          succeed = false;
        }
      }
    }
  }
  return succeed;
}

// FIXME: move to HttpContext class
// return false if any error
bool parseRequest(Buffer* buf, HttpContext* context, Timestamp receiveTime)
{
  bool ok = true;
  bool hasMore = true;
  while (hasMore)
  {
    if (context->expectRequestLine())
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        ok = processRequestLine(buf->peek(), crlf, context);
        if (ok)
        {
          context->request().setReceiveTime(receiveTime);
          buf->retrieveUntil(crlf + 2);
          context->receiveRequestLine();
        }
        else
        {
          hasMore = false;
        }
      }
      else
      {
        hasMore = false;
      }
    }
    else if (context->expectHeaders())
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        const char* colon = std::find(buf->peek(), crlf, ':');
        if (colon != crlf)
        {
          context->request().addHeader(buf->peek(), colon, crlf);
        }
        else
        {
          // empty line, end of header
          context->receiveHeaders();
          hasMore = !context->gotAll();
        }
        buf->retrieveUntil(crlf + 2);
      }
      else
      {
        hasMore = false;
      }
    }
    else if (context->expectBody())
    {
      // FIXME:
    }
  }
  return ok;
}

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}

}
}
}

HttpServer::HttpServer(EventLoop* loop, const InetAddress& listenAddr, const string& name)
  : m_server(loop, listenAddr, name),
    m_httpCallback(detail::defaultHttpCallback)
{
  m_server.setConnectionCallback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
  m_server.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

HttpServer::~HttpServer()
{
}

void HttpServer::start()
{
  LOG_WARN << "HttpServer[" << m_server.name() << "] starts listenning on " << m_server.hostport();
  //std::cout << "HttpServer[" << m_server.name() << "] starts listenning on " << m_server.hostport();
  m_server.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    conn->setContext(HttpContext());
  }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
{
  //HttpContext* context = any_cast<HttpContext>(conn->getMutableContext());
	HttpContext* context = any_cast<HttpContext*>(conn->getContext());

  if (!detail::parseRequest(buf, context, receiveTime))
  {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }

  if (context->gotAll())
  {
    onRequest(conn, context->request());
    context->reset();
  }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
  const string& connection = req.getHeader("Connection");
  bool close = connection == "close" || (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  HttpResponse response(close);
  m_httpCallback(req, &response);
  Buffer buf;
  response.appendToBuffer(&buf);
  conn->send(&buf);
  if (response.closeConnection())
  {
    conn->shutdown();
  }
}

