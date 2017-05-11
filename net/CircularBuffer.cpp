#include <errno.h>
#include <stdio.h>
#include <sys/uio.h>
#include "CircularBuffer.h"
#include "SocketOps.h"

using namespace kimgbo;
using namespace kimgbo::net;
	
const char CircularBuffer::kCRLF[] = "\r\n";

const size_t CircularBuffer::kCheapPrepend;
const size_t CircularBuffer::kInitialSize;
	
ssize_t CircularBuffer::readFd(int fd, int* savedErrno)
{
  // saved an ioctl()/FIONREAD call to tell how much to read
  char extrabuf[65536];
  struct iovec vec[2];
  size_t writable;
  if(m_writerIndex > m_readerIndex)
  {
  	writable = m_buffer.size() - m_writerIndex;
  }
  else
  {
  	writable = writableBytes();
  }
  
  vec[0].iov_base = begin()+m_writerIndex;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  const ssize_t n = sockets::readv(fd, vec, 2);
  if (n < 0)
  {
    *savedErrno = errno;
  }
  else if (implicit_cast<size_t>(n) <= writable)
  {
    m_writerIndex += n;
    m_hasData = true;
  }
  else
  {
    m_writerIndex = m_buffer.size();
    append(extrabuf, n - writable);
  }
  // if (n == writable + sizeof extrabuf)
  // {
  //   goto line_30;
  // }
  printf("buffer_size: %d, writable: %d, n: %d\n", m_buffer.size(), n, writable);
  return n;
}