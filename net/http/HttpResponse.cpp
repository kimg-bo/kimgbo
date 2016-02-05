#include <stdio.h>
#include "HttpResponse.h"
#include "Buffer.h"

using namespace kimgbo;
using namespace kimgbo::net;

void HttpResponse::appendToBuffer(Buffer* output) const
{
  char buf[32];
  snprintf(buf, sizeof buf, "HTTP/1.1 %d ", m_statusCode);
  output->append(buf);
  output->append(m_statusMessage);
  output->append("\r\n");

  if (m_closeConnection)
  {
    output->append("Connection: close\r\n");
  }
  else
  {
    snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", m_body.size());
    output->append(buf);
    output->append("Connection: Keep-Alive\r\n");
  }

  for (std::map<string, string>::const_iterator it = m_headers.begin();
       it != m_headers.end();
       ++it)
  {
    output->append(it->first);
    output->append(": ");
    output->append(it->second);
    output->append("\r\n");
  }

  output->append("\r\n");
  output->append(m_body);
}
