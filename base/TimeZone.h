#ifndef BASE_TIMEZONE_H
#define BASE_TIMEZONE_H

#include <time.h>
#include <tr1/memory>

namespace kimgbo
{

// TimeZone for 1970~2030
class TimeZone
{
 public:
  explicit TimeZone(const char* zonefile);

  // default copy ctor/assignment/dtor are Okay.

  bool valid() const { return m_data; }
  struct tm toLocalTime(time_t secondsSinceEpoch) const;
  time_t fromLocalTime(const struct tm&) const;

  // gmtime(3)
  static struct tm toUtcTime(time_t secondsSinceEpoch, bool yday = false);
  // timegm(3)
  static time_t fromUtcTime(const struct tm&);
  // year in [1900..2500], month in [1..12], day in [1..31]
  static time_t fromUtcTime(int year, int month, int day,
                            int hour, int minute, int seconds);

  struct Data;

 private:

  std::tr1::shared_ptr<Data> m_data;
};

}
#endif  // MUDUO_BASE_TIMEZONE_H
