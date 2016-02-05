#include <assert.h>
#include <stdio.h>
#include <time.h>
#include "LogFile.h"
#include "Logging.h" // strerror_tl
#include "ProcessInfo.h"

using namespace kimgbo;

// not thread safe
class LogFile::File
{
 public:
  explicit File(const std::string& filename)
    : m_fp(::fopen(filename.data(), "ae")),
      m_writtenBytes(0)
  {
    assert(m_fp);
    ::setbuffer(m_fp, m_buffer, sizeof(m_buffer));
    // posix_fadvise POSIX_FADV_DONTNEED ?
  }

  ~File()
  {
    ::fclose(m_fp);
  }

  void append(const char* logline, const size_t len)
  {
    size_t n = write(logline, len);
    size_t remain = len - n;
    while (remain > 0)
    {
      size_t x = write(logline + n, remain);
      if (x == 0)
      {
        int err = ferror(m_fp);
        if (err)
        {
          fprintf(stderr, "LogFile::File::append() failed %s\n", strerror_tl(err));
        }
        break;
      }
      n += x;
      remain = len - n; // remain -= x
    }

    m_writtenBytes += len;
  }

  void flush()
  {
    ::fflush(m_fp);
  }

  size_t writtenBytes() const { return m_writtenBytes; }

 private:

  size_t write(const char* logline, size_t len)
  {
#undef fwrite_unlocked
    return ::fwrite_unlocked(logline, 1, len, m_fp);
  }

  FILE* m_fp;
  char m_buffer[64*1024];
  size_t m_writtenBytes;
};

LogFile::LogFile(const std::string& basename,
                 size_t rollSize,
                 bool threadSafe,
                 int flushInterval)
  : m_basename(basename),
    krollSize(rollSize),
    kflushInterval(flushInterval),
    m_count(0),
    m_mutex(threadSafe ? new MutexLock : NULL),
    m_startOfPeriod(0),
    m_lastRoll(0),
    m_lastFlush(0)
{
  assert(basename.find('/') == std::string::npos);
  rollFile();
}

LogFile::~LogFile()
{
}

void LogFile::append(const char* logline, int len)
{
  if (m_mutex)
  {
    MutexLockGuard lock(*m_mutex);
    append_unlocked(logline, len);
  }
  else
  {
    append_unlocked(logline, len);
  }
}

void LogFile::flush()
{
  if (m_mutex)
  {
    MutexLockGuard lock(*m_mutex);
    m_file->flush();
  }
  else
  {
    m_file->flush();
  }
}

void LogFile::append_unlocked(const char* logline, int len)
{
  m_file->append(logline, len);

  if (m_file->writtenBytes() > krollSize)
  {
    rollFile();
  }
  else
  {
    if (m_count > m_kCheckTimeRoll)
    {
      m_count = 0;
      time_t now = ::time(NULL);
      time_t thisPeriod_ = now / m_kRollPerSeconds * m_kRollPerSeconds;
      if (thisPeriod_ != m_startOfPeriod)
      {
        rollFile();
      }
      else if (now - m_lastFlush > kflushInterval)
      {
        m_lastFlush = now;
        m_file->flush();
      }
    }
    else
    {
      ++m_count;
    }
  }
}

void LogFile::rollFile()
{
  time_t now = 0;
  std::string filename = getLogFileName(m_basename, &now);
  time_t start = now / m_kRollPerSeconds * m_kRollPerSeconds;

  if (now > m_lastRoll)
  {
    m_lastRoll = now;
    m_lastFlush = now;
    m_startOfPeriod = start;
    m_file.reset(new File(filename));
  }
}

std::string LogFile::getLogFileName(const std::string& basename, time_t* now)
{
  std::string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  char pidbuf[32];
  struct tm tm;
  *now = time(NULL);
  gmtime_r(now, &tm); // FIXME: localtime_r ?
  strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S.", &tm);
  filename += timebuf;
  filename += ProcessInfo::hostname();
  snprintf(pidbuf, sizeof(pidbuf), ".%d", ProcessInfo::pid());
  filename += pidbuf;
  filename += ".log";

  return filename;
}

