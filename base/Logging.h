#ifndef BASE_LOGGING_H
#define BASE_LOGGING_H

#include "LogStream.h"
#include "Timestamp.h"

namespace kimgbo
{

class Logger
{
 public:
  enum LogLevel
  {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };

  // compile time calculation of basename of source file
  class SourceFile
  {
   public:
    template<int N>
    inline SourceFile(const char (&arr)[N])
      : m_data(arr),
        m_size(N-1)
    {
      const char* slash = strrchr(m_data, '/'); // builtin function
      if (slash)
      {
        m_data = slash + 1;
        m_size -= static_cast<int>(m_data - arr);
      }
    }

    explicit SourceFile(const char* filename)
      : m_data(filename)
    {
      const char* slash = strrchr(filename, '/');
      if (slash)
      {
        m_data = slash + 1;
      }
      m_size = static_cast<int>(strlen(m_data));
    }

    const char* m_data;
    int m_size;
  };

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char* func);
  Logger(SourceFile file, int line, bool toAbort);
  ~Logger();

  LogStream& stream() { return m_impl.m_stream; }

  static LogLevel logLevel();
  static void setLogLevel(LogLevel level);

  typedef void (*OutputFunc)(const char* msg, int len);
  typedef void (*FlushFunc)();
  static void setOutput(OutputFunc);
  static void setFlush(FlushFunc);

 private:

class Impl
{
 public:
  typedef Logger::LogLevel LogLevel;
  Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
  void formatTime();
  void finish();

  Timestamp m_time;
  LogStream m_stream;
  LogLevel m_level;
  int m_line;
  SourceFile m_basename;
};

  Impl m_impl;

};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
  return g_logLevel;
}

#define LOG_TRACE if (kimgbo::Logger::logLevel() <= kimgbo::Logger::TRACE) \
  kimgbo::Logger(__FILE__, __LINE__, kimgbo::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (kimgbo::Logger::logLevel() <= kimgbo::Logger::DEBUG) \
  kimgbo::Logger(__FILE__, __LINE__, kimgbo::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (kimgbo::Logger::logLevel() <= kimgbo::Logger::INFO) \
  kimgbo::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN kimgbo::Logger(__FILE__, __LINE__, kimgbo::Logger::WARN).stream()
#define LOG_ERROR kimgbo::Logger(__FILE__, __LINE__, kimgbo::Logger::ERROR).stream()
#define LOG_FATAL kimgbo::Logger(__FILE__, __LINE__, kimgbo::Logger::FATAL).stream()
#define LOG_SYSERR kimgbo::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL kimgbo::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor
// initializer lists.

#define CHECK_NOTNULL(val) \
  ::kimgbo::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr) {
  if (ptr == NULL) {
   Logger(file, line, Logger::FATAL).stream() << names;
  }
  return ptr;
}

}

#endif  // BASE_LOGGING_H
