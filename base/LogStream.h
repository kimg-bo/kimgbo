#ifndef BASE_LOGSTREAM_H
#define BASE_LOGSTREAM_H

#include <assert.h>
#include <string.h> // memcpy
#ifndef MUDUO_STD_STRING
#include <string>
#endif
#include "StringPiece.h"
#include "Types.h"

namespace kimgbo
{

namespace detail
{

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000*1000;

template<int SIZE>
class FixedBuffer
{
 public:
  FixedBuffer()
    : m_cur(m_data)
  {
    setCookie(cookieStart);
  }

  ~FixedBuffer()
  {
    setCookie(cookieEnd);
  }

  void append(const char* /*restrict*/ buf, size_t len)
  {
    // FIXME: append partially
    if (implicit_cast<size_t>(avail()) > len)
    {
      memcpy(m_cur, buf, len);
      m_cur += len;
    }
  }

  const char* data() const { return m_data; }
  int length() const { return static_cast<int>(m_cur - m_data); }

  // write to data_ directly
  char* current() { return m_cur; }
  int avail() const { return static_cast<int>(end() - m_cur); }
  void add(size_t len) { m_cur += len; }

  void reset() { m_cur = m_data; }
  void bzero() { ::bzero(m_data, sizeof(m_data)); }

  // for used by GDB
  const char* debugString();
  void setCookie(void (*cookie)()) { m_cookie = cookie; }
  // for used by unit test
  string asString() const { return string(m_data, length()); }

 private:
  const char* end() const { return m_data + sizeof(m_data); }
  // Must be outline function for cookies.
  static void cookieStart();
  static void cookieEnd();

  void (*m_cookie)();
  char m_data[SIZE];
  char* m_cur;
};

}

class LogStream
{
  typedef LogStream self;
 public:
  typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

  self& operator<<(bool v)
  {
    m_buffer.append(v ? "1" : "0", 1);
    return *this;
  }

  self& operator<<(short);
  self& operator<<(unsigned short);
  self& operator<<(int);
  self& operator<<(unsigned int);
  self& operator<<(long);
  self& operator<<(unsigned long);
  self& operator<<(long long);
  self& operator<<(unsigned long long);

  self& operator<<(const void*);

  self& operator<<(float v)
  {
    *this << static_cast<double>(v);
    return *this;
  }
  self& operator<<(double);
  // self& operator<<(long double);

  self& operator<<(char v)
  {
    m_buffer.append(&v, 1);
    return *this;
  }

  // self& operator<<(signed char);
  // self& operator<<(unsigned char);

  self& operator<<(const char* v)
  {
    m_buffer.append(v, strlen(v));
    return *this;
  }
  
  //self& operator<<(char* v)
  //{
  //  m_buffer.append(v, strlen(v));
  //  return *this;
  //}

  self& operator<<(const string& v)
  {
    m_buffer.append(v.c_str(), v.size());
    return *this;
  }

#ifndef MUDUO_STD_STRING
  self& operator<<(const std::string& v)
  {
    m_buffer.append(v.c_str(), v.size());
    return *this;
  }
#endif

  self& operator<<(const StringPiece& v)
  {
    m_buffer.append(v.data(), v.size());
    return *this;
  }

  void append(const char* data, int len) { m_buffer.append(data, len); }
  const Buffer& buffer() const { return m_buffer; }
  void resetBuffer() { m_buffer.reset(); }

 private:
  void staticCheck();

  template<typename T>
  void formatInteger(T);

  Buffer m_buffer;

  static const int kMaxNumericSize = 32;
};

class Fmt // : boost::noncopyable
{
 public:
  template<typename T>
  Fmt(const char* fmt, T val);

  const char* data() const { return m_buf; }
  int length() const { return m_length; }

 private:
  char m_buf[32];
  int m_length;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
  s.append(fmt.data(), fmt.length());
  return s;
}

}
#endif  // BASE_LOGSTREAM_H

