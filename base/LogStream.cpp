#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <algorithm>
#include <limits>
#include <cassert>
#include "LogStream.h"

using namespace kimgbo;
using namespace kimgbo::detail;

#pragma GCC diagnostic ignored "-Wtype-limits"
//#pragma GCC diagnostic error "-Wtype-limits"
namespace kimgbo
{
namespace detail
{

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

static_assert(sizeof(digits) == 20, "sizeof(digits) != 20");

const char digitsHex[] = "0123456789ABCDEF";
static_assert(sizeof(digitsHex) == 17, "sizeof digitsHex != 17");

// Efficient Integer to String Conversions, by Matthew Wilson.
template<typename T>
size_t convert(char buf[], T value)
{
  T i = value;
  char* p = buf;

  do
  {
    int lsd = static_cast<int>(i % 10);
    i /= 10;
    *p++ = zero[lsd];
  } while (i != 0);

  if (value < 0)
  {
    *p++ = '-';
  }
  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

size_t convertHex(char buf[], uintptr_t value)
{
  uintptr_t i = value;
  char* p = buf;

  do
  {
    int lsd = i % 16;
    i /= 16;
    *p++ = digitsHex[lsd];
  } while (i != 0);

  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

}
}

template<int SIZE>
const char* FixedBuffer<SIZE>::debugString()
{
  *m_cur = '\0';
  return m_data;
}

template<int SIZE>
void FixedBuffer<SIZE>::cookieStart()
{
}

template<int SIZE>
void FixedBuffer<SIZE>::cookieEnd()
{
}

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

void LogStream::staticCheck()
{
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10, "kMaxNumericSize - 10 !> std::numeric_limits<double>::digits10");
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10, "kMaxNumericSize - 10 !> std::numeric_limits<long double>::digits10");
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10, "kMaxNumericSize - 10 !> std::numeric_limits<long>::digits10");
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10, "kMaxNumericSize - 10 !> std::numeric_limits<long long>::digits10");
}

template<typename T>
void LogStream::formatInteger(T v)
{
  if (m_buffer.avail() >= kMaxNumericSize)
  {
    size_t len = convert(m_buffer.current(), v);
    m_buffer.add(len);
  }
}

LogStream& LogStream::operator<<(short v)
{
  *this << static_cast<int>(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
  *this << static_cast<unsigned int>(v);
  return *this;
}

LogStream& LogStream::operator<<(int v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(long long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator<<(const void* p)
{
  uintptr_t v = reinterpret_cast<uintptr_t>(p);
  if (m_buffer.avail() >= kMaxNumericSize)
  {
    char* buf = m_buffer.current();
    buf[0] = '0';
    buf[1] = 'x';
    size_t len = convertHex(buf+2, v);
    m_buffer.add(len+2);
  }
  return *this;
}

// FIXME: replace this with Grisu3 by Florian Loitsch.
LogStream& LogStream::operator<<(double v)
{
  if (m_buffer.avail() >= kMaxNumericSize)
  {
    int len = snprintf(m_buffer.current(), kMaxNumericSize, "%.12g", v);
    m_buffer.add(len);
  }
  return *this;
}

template<typename T>
Fmt::Fmt(const char* fmt, T val)
{
  static_assert(std::is_arithmetic<T>::value == true, "std::is_arithmetic<T>::value != true");

  m_length = snprintf(m_buf, sizeof(m_buf), fmt, val);
  assert(static_cast<size_t>(m_length) < sizeof(m_buf));
}

// Explicit instantiations

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);
