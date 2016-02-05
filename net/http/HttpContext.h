#ifndef HTTP_HTTPCONTEXT_H
#define HTTP_HTTPCONTEXT_H

#include "HttpRequest.h"

namespace kimgbo
{
namespace net
{

class HttpContext
{
 public:
  enum HttpRequestParseState
  {
    kExpectRequestLine,
    kExpectHeaders,
    kExpectBody,
    kGotAll,
  };

  HttpContext()
    : m_state(kExpectRequestLine)
  {
  }

  // default copy-ctor, dtor and assignment are fine

  bool expectRequestLine() const
  { return m_state == kExpectRequestLine; }

  bool expectHeaders() const
  { return m_state == kExpectHeaders; }

  bool expectBody() const
  { return m_state == kExpectBody; }

  bool gotAll() const
  { return m_state == kGotAll; }

  void receiveRequestLine()
  { m_state = kExpectHeaders; }

  void receiveHeaders()
  { m_state = kGotAll; }  // FIXME

  void reset()
  {
    m_state = kExpectRequestLine;
    HttpRequest dummy;
    m_request.swap(dummy);
  }

  const HttpRequest& request() const
  { return m_request; }

  HttpRequest& request()
  { return m_request; }

 private:
  HttpRequestParseState m_state;
  HttpRequest m_request;
};

}
}

#endif  // MUDUO_NET_HTTP_HTTPCONTEXT_H
