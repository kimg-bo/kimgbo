#ifndef HTTP_HTTPRESPONSE_H
#define HTTP_HTTPRESPONSE_H

#include <map>
#include "Buffer.h"
#include "Types.h"

namespace kimgbo
{
namespace net
{

//class Buffer;
class HttpResponse
{
 public:
  enum HttpStatusCode
  {
    kUnknown,
    k200Ok = 200,
    k301MovedPermanently = 301,
    k400BadRequest = 400,
    k404NotFound = 404,
  };

  explicit HttpResponse(bool close)
    : m_statusCode(kUnknown),
      m_closeConnection(close)
  {
  }

  void setStatusCode(HttpStatusCode code)
  { m_statusCode = code; }

  void setStatusMessage(const string& message)
  { m_statusMessage = message; }

  void setCloseConnection(bool on)
  { m_closeConnection = on; }

  bool closeConnection() const
  { return m_closeConnection; }

  void setContentType(const string& contentType)
  { addHeader("Content-Type", contentType); }

  // FIXME: replace string with StringPiece
  void addHeader(const string& key, const string& value)
  { m_headers[key] = value; }

  void setBody(const std::string& body)
  { m_body = body; }

  void appendToBuffer(Buffer* output) const;

 private:
  std::map<string, string> m_headers;
  HttpStatusCode m_statusCode;
  // FIXME: add http version
  string m_statusMessage;
  bool m_closeConnection;
  std::string m_body;
};

}
}

#endif  // MUDUO_NET_HTTP_HTTPRESPONSE_H
