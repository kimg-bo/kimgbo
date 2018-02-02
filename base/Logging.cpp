#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include "Logging.h"
#include "CurrentThread.h"
#include "StringPiece.h"
#include "Timestamp.h"

namespace kimgbo
{

__thread char t_errnobuf[512];
__thread char t_time[32];
__thread time_t t_lastSecond;

const char* strerror_tl(int savedErrno)
{
  return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

Logger::LogLevel initLogLevel()
{
  if (::getenv("MUDUO_LOG_TRACE"))
    return Logger::TRACE;
  else if (::getenv("MUDUO_LOG_DEBUG"))
    return Logger::DEBUG;
  else
    return Logger::DEBUG;
}

Logger::LogLevel g_logLevel = initLogLevel();

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL ",
};

// helper class for known string length at compile time
class T
{
 public:
  T(const char* str, unsigned len)
    :m_str(str),
     m_len(len)
  {
    assert(strlen(str) == m_len);
  }

  const char* m_str;
  const unsigned m_len;
};

inline LogStream& operator<<(LogStream& s, T v)
{
  s.append(v.m_str, v.m_len);
  return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)
{
  s.append(v.m_data, v.m_size);
  return s;
}

void defaultOutput(const char* msg, int len)
{
  size_t n = fwrite(msg, 1, len, stdout);
  //FIXME check n
  (void)n;
}

void defaultFlush()
{
  fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

}

using namespace kimgbo;

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile& file, int line)
  : m_time(Timestamp::now()),
    m_stream(),
    m_level(level),
    m_line(line),
    m_basename(file)
{
  formatTime();
  CurrentThread::tid();
  m_stream << T(CurrentThread::tidString(), 6);
  m_stream << T(LogLevelName[level], 6);
  if (savedErrno != 0)
  {
    m_stream << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
  }
}

void Logger::Impl::formatTime()
{
  int64_t microSecondsSinceEpoch = m_time.microSecondsSinceEpoch();
  time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / 1000000);
  int microseconds = static_cast<int>(microSecondsSinceEpoch % 1000000);
  if (seconds != t_lastSecond)
  {
    t_lastSecond = seconds;
    struct tm tm_time;
    ::gmtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime

    int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
        tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
        tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    assert(len == 17); (void)len;
  }
  Fmt us(".%06dZ ", microseconds);
  assert(us.length() == 9);
  m_stream << T(t_time, 17) << T(us.data(), 9);
}

void Logger::Impl::finish()
{
  m_stream << " - " << m_basename << ':' << m_line << '\n';
}

Logger::Logger(SourceFile file, int line)
  : m_impl(INFO, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
  : m_impl(level, 0, file, line)
{
  m_impl.m_stream << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
  : m_impl(level, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, bool toAbort)
  : m_impl(toAbort?FATAL:ERROR, errno, file, line)
{
}

Logger::~Logger()
{
  m_impl.finish();
  const LogStream::Buffer& buf(stream().buffer());
  g_output(buf.data(), buf.length());
  if (m_impl.m_level == FATAL)
  {
    g_flush();
    abort();
  }
}

void Logger::setLogLevel(Logger::LogLevel level)
{
  g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
  g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
  g_flush = flush;
}
