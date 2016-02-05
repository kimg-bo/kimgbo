#include <errno.h>
#include <stdio.h>
#include <functional>
#include "TcpConnection.h"
#include "StringPiece.h"
#include "Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"
#include "SocketOps.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
void kimgbo::net::defaultConnectionCallback(const TcpConnectionPtr& conn)
{
	LOG_TRACE << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
}

void kimgbo::net::defaultMessageCallback(const TcpConnectionPtr&, Buffer* buf, Timestamp)
{
  buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop* loop, const string& nameArg, int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr)
  : m_loop(CHECK_NOTNULL(loop)),
    m_name(nameArg),
    m_state(kConnecting),
    m_socket(new Socket(sockfd)),
    m_channel(new Channel(loop, sockfd)),
    m_localAddr(localAddr),
    m_peerAddr(peerAddr),
    m_highWaterMark(64*1024*1024),
    m_context(NULL)
{
  m_channel->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
  m_channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  m_channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
  m_channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));
 
  LOG_DEBUG << "TcpConnection::ctor[" <<  m_name << "] at " << this << " fd=" << sockfd;
 
  m_socket->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
  LOG_DEBUG << "TcpConnection::dtor[" <<  m_name << "] at " << this << " fd=" << m_channel->fd();
}

void TcpConnection::send(const void* data, size_t len)
{
  if (m_state == kConnected)
  {
    if (m_loop->isInLoopThread())
    {
      //sendInLoop(data, len);
      sendInLoopMessage(data, len);
    }
    else
    {
      string message(static_cast<const char*>(data), len);
      m_loop->runInLoop(std::bind(&TcpConnection::sendInLoopStringPiece, this, message));
    }
  }
}

void TcpConnection::send(const StringPiece& message)
{
  if (m_state == kConnected)
  {
    if (m_loop->isInLoopThread())
    {
      //sendInLoop(message);
      sendInLoopStringPiece(message);
    }
    else
    {
      m_loop->runInLoop(std::bind(&TcpConnection::sendInLoopStringPiece, this, message.as_string()));
                    //std::forward<string>(message)));
    }
  }
}
void TcpConnection::send(Buffer* buf)
{
	if(m_state == kConnected)
	{
		if(m_loop->isInLoopThread())
		{
			sendInLoopMessage(buf->peek(), buf->readableBytes());
			buf->retrieveAll();
		}
		else
		{
			m_loop->runInLoop(std::bind(&TcpConnection::sendInLoopStringPiece, this, buf->retrieveAllAsString()));
		}
	}
}

void TcpConnection::sendInLoopStringPiece(const StringPiece& message)
{
  sendInLoopMessage(message.data(), message.size());
}

void TcpConnection::sendInLoopMessage(const void* data, size_t len)
{
  m_loop->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool error = false;
  if (m_state == kDisconnected)
  {
    LOG_WARN << "disconnected, give up writing";
    return;
  }
  // if no thing in output queue, try writing directly
  if (!m_channel->isWriting() && m_outputBuffer.readableBytes() == 0)
  {
    nwrote = sockets::write(m_channel->fd(), data, len);
    if (nwrote >= 0)
    {
      remaining = len - nwrote;
      if (remaining == 0 && m_writeCompleteCallback)
      {
        m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
        	LOG_INFO << "will call m_writeCompleteCallback";
      }
    }
    else // nwrote < 0
    {
      nwrote = 0;
      if (errno != EWOULDBLOCK)
      {
        LOG_SYSERR << "TcpConnection::sendInLoop";
        if (errno == EPIPE) // FIXME: any others?
        {
          error = true;
        }
      }
    }
  }

  assert(remaining <= len);
  if (!error && remaining > 0)
  {
    LOG_TRACE << "I am going to write more data";
   
    size_t oldLen = m_outputBuffer.readableBytes();
    if (oldLen + remaining >= m_highWaterMark
        && oldLen < m_highWaterMark
        && m_highWaterMarkCallback)
    {
      m_loop->queueInLoop(std::bind(m_highWaterMarkCallback, shared_from_this(), oldLen + remaining));
    }
    m_outputBuffer.append(static_cast<const char*>(data)+nwrote, remaining);
    if (!m_channel->isWriting())
    {
      m_channel->enableWriting();
    }
  }
}

void TcpConnection::shutdown()
{
  // FIXME: use compare and swap
  if (m_state == kConnected)
  {
    setState(kDisconnecting);
    // FIXME: shared_from_this()?
    m_loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
  }
}

void TcpConnection::shutdownInLoop()
{
  m_loop->assertInLoopThread();
  if (!m_channel->isWriting())
  {
    // we are not writing
    m_socket->shutdownWrite();
  }
}

void TcpConnection::setTcpNoDelay(bool on)
{
  m_socket->setTcpNoDelay(on);
}

void TcpConnection::connectEstablished()
{
  m_loop->assertInLoopThread();
  assert(m_state == kConnecting);
  setState(kConnected);
  m_channel->tie(shared_from_this());
  m_channel->enableReading();

  m_connectionCallback(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
  m_loop->assertInLoopThread();
  if (m_state == kConnected)
  {
    setState(kDisconnected);
    m_channel->disableAll();

    m_connectionCallback(shared_from_this());
  }
  m_channel->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
  m_loop->assertInLoopThread();
  int savedErrno = 0;
  ssize_t n = m_inputBuffer.readFd(m_channel->fd(), &savedErrno);
  if (n > 0)
  {
    m_messageCallback(shared_from_this(), &m_inputBuffer, receiveTime);
  }
  else if (n == 0)
  {
    handleClose();
  }
  else
  {
    errno = savedErrno;
    LOG_SYSERR << "TcpConnection::handleRead";
    handleError();
  }
}

void TcpConnection::handleWrite()
{
  m_loop->assertInLoopThread();
  if (m_channel->isWriting())
  {
    ssize_t n = sockets::write(m_channel->fd(), m_outputBuffer.peek(), m_outputBuffer.readableBytes());
    if (n > 0)
    {
      m_outputBuffer.retrieve(n);
      if (m_outputBuffer.readableBytes() == 0)
      {
        m_channel->disableWriting();
        if (m_writeCompleteCallback)
        {
          m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
        }
        if (m_state == kDisconnecting)
        {
          shutdownInLoop();
        }
      }
      else
      {
        LOG_TRACE << "I am going to write more data";
      }
    }
    else
    {
      LOG_SYSERR << "TcpConnection::handleWrite";
      // if (state_ == kDisconnecting)
      // {
      //   shutdownInLoop();
      // }
    }
  }
  else
  {
    LOG_TRACE << "Connection fd = " << m_channel->fd()
              << " is down, no more writing";
  }
}

void TcpConnection::handleClose()
{
  m_loop->assertInLoopThread();
  LOG_TRACE << "fd = " << m_channel->fd() << " state = " << m_state;
  
  assert(m_state == kConnected || m_state == kDisconnecting);
  // we don't close fd, leave it to dtor, so we can find leaks easily.
  setState(kDisconnected);
  m_channel->disableAll();

  TcpConnectionPtr guardThis(shared_from_this());
  m_connectionCallback(guardThis);
  // must be the last line
  m_closeCallback(guardThis);
}

void TcpConnection::handleError()
{
  int err = sockets::getSocketError(m_channel->fd());
  LOG_ERROR << "TcpConnection::handleError [" << m_name
            << "] - SO_ERROR = " << err << " " << strerror(err);
}