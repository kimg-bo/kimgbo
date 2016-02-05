#ifndef EXAMPLES_ASIO_CHAT_CODEC_H
#define EXAMPLES_ASIO_CHAT_CODEC_H

//#include <muduo/base/Logging.h>
#include <functional>
#include "Buffer.h"
#include "Endian.h"
#include "TcpConnection.h"


class LengthHeaderCodec
{
 public:
  typedef std::function<void (const kimgbo::net::TcpConnectionPtr&,
                                const kimgbo::string& message,
                                kimgbo::Timestamp)> StringMessageCallback;

  explicit LengthHeaderCodec(const StringMessageCallback& cb)
    : m_messageCallback(cb)
  {
  }

  void onMessage(const kimgbo::net::TcpConnectionPtr& conn,
                 kimgbo::net::Buffer* buf,
                 kimgbo::Timestamp receiveTime)
  {
    while (buf->readableBytes() >= kHeaderLen) // kHeaderLen == 4
    {
      // FIXME: use Buffer::peekInt32()
      const void* data = buf->peek();
      int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
      const int32_t len = kimgbo::net::sockets::networkToHost32(be32);
      if (len > 65536 || len < 0)
      {
        //LOG_ERROR << "Invalid length " << len;
        std::cout << "Invalid length " << len;
        conn->shutdown();  // FIXME: disable reading
        break;
      }
      else if (buf->readableBytes() >= len + kHeaderLen)
      {
        buf->retrieve(kHeaderLen);
        kimgbo::string message(buf->peek(), len);
        m_messageCallback(conn, message, receiveTime);
        buf->retrieve(len);
      }
      else
      {
        break;
      }
    }
  }

  // FIXME: TcpConnectionPtr
  void send(kimgbo::net::TcpConnection* conn,
            const kimgbo::StringPiece& message)
  {
    kimgbo::net::Buffer buf;
    buf.append(message.data(), message.size());
    int32_t len = static_cast<int32_t>(message.size());
    int32_t be32 = kimgbo::net::sockets::hostToNetwork32(len);
    buf.prepend(&be32, sizeof be32);
    conn->send(&buf);
  }

 private:
  StringMessageCallback m_messageCallback;
  const static size_t kHeaderLen = sizeof(int32_t);
};

#endif  // MUDUO_EXAMPLES_ASIO_CHAT_CODEC_H
