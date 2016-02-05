#ifndef BASE_LOGFILE_H
#define BASE_LOGFILE_H

#include <memory>
#include <string>
#include "Mutex.h"
//#include "Types.h"


namespace kimgbo
{

class LogFile
{
 public:
  LogFile(const std::string& basename,
          size_t rollSize,
          bool threadSafe = true,
          int flushInterval = 3);
  ~LogFile();

  void append(const char* logline, int len);
  void flush();

 private:
  void append_unlocked(const char* logline, int len);

  static std::string getLogFileName(const std::string& basename, time_t* now);
  void rollFile();

  const std::string m_basename;
  const size_t krollSize;
  const int kflushInterval;

  int m_count;

  std::unique_ptr<MutexLock> m_mutex;
  time_t m_startOfPeriod;
  time_t m_lastRoll;
  time_t m_lastFlush;
  class File;
  std::unique_ptr<File> m_file;

  const static int m_kCheckTimeRoll = 1024;
  const static int m_kRollPerSeconds = 60*60*24;
};

}
#endif  // BASE_LOGFILE_H
