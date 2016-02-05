#ifndef CHAT_CODEC_H
#define CHAT_CODEC_H

#include <functional>
#include "Logging.h"
#include "Buffer.h"
#include "Endian.h"
#include "TcpConnection.h"

class LengthHeaderCodec
{
public:
	typedef std::function<void (const kimgbo::net::TcpConnectionPtr& conn, 
												const kimgbo::string& message, kimgbo::Timestamp time_)> StringMessageCallback;
													
	explicit LengthHeaderCodec(const StringMessageCallback& cb)
		: m_stringMessageCallback(cb)
	{
	}
	
	void onMessage(const kimgbo::net::TcpConnectionPtr& conn,
                 kimgbo::net::Buffer* buf, kimgbo::Timestamp receiveTime)
  {
  	while(buf->readableBytes() >= kHeaderLen)
  	{
  		const void* data = buf->peek();
  		int32_t be32 = *static_cast<const int32_t*>(data);
  		const int32_t len = kimgbo::net::sockets::networkToHost32(be32);
  		if(len > 65536 || len < 0)
  		{
  			LOG_ERROR << "Invalid length " << len;
        conn->shutdown();  // FIXME: disable reading
        break;
  		}
  		else if(buf->readableBytes() >= kHeaderLen + len)
  		{
  			buf->retrieve(kHeaderLen);
  			kimgbo::string msg(buf->peek(), len);
  			m_stringMessageCallback(conn, msg, receiveTime);
  			buf->retrieve(len);
  		}
  		else
  		{
  			break;
  		}
  	}
  }
  
  void send(kimgbo::net::TcpConnection* conn, const kimgbo::StringPiece& message)
  {
  	kimgbo::net::Buffer buf;
  	buf.append(message.data(), message.size());
  	int32_t len = static_cast<int32_t>(message.size());
  	int32_t be32 = kimgbo::net::sockets::hostToNetwork32(len);
  	buf.prepend(&be32, sizeof(be32));
  	conn->send(&buf);
  }	
	
private:
	StringMessageCallback m_stringMessageCallback;
	const static size_t kHeaderLen = sizeof(int32_t);
};

#endif //CHAT_CODEC_H