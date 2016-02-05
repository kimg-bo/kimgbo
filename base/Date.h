// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//

#ifndef BASE_DATE_H
#define BASE_DATE_H

#include "Types.h"

struct tm;

namespace kimgbo
{

///
/// Date in Gregorian calendar.
///
/// This class is immutable.
/// It's recommended to pass it by value, since it's passed in register on x64.
///
class Date
          // public boost::less_than_comparable<Date>,
          // public boost::equality_comparable<Date>
{
 public:

  struct YearMonthDay
  {
    int year; // [1900..2500]
    int month;  // [1..12]
    int day;  // [1..31]
  };

  static const int kDaysPerWeek = 7;
  static const int kJulianDayOf1970_01_01;

  ///
  /// Constucts an invalid Date.
  ///
  Date()
    : m_julianDayNumber(0)
  {}

  ///
  /// Constucts a yyyy-mm-dd Date.
  ///
  /// 1 <= month <= 12
  Date(int year, int month, int day);

  ///
  /// Constucts a Date from Julian Day Number.
  ///
  explicit Date(int julianDayNum)
    : m_julianDayNumber(julianDayNum)
  {}

  ///
  /// Constucts a Date from struct tm
  ///
  explicit Date(const struct tm&);

  // default copy/assignment/dtor are Okay

  void swap(Date& that)
  {
    std::swap(m_julianDayNumber, that.m_julianDayNumber);
  }

  bool valid() const { return m_julianDayNumber > 0; }

  ///
  /// Converts to yyyy-mm-dd format.
  ///
  string toIsoString() const;

  struct YearMonthDay yearMonthDay() const;

  int year() const
  {
    return yearMonthDay().year;
  }

  int month() const
  {
    return yearMonthDay().month;
  }

  int day() const
  {
    return yearMonthDay().day;
  }

  // [0, 1, ..., 6] => [Sunday, Monday, ..., Saturday ]
  int weekDay() const
  {
    return (m_julianDayNumber+1) % kDaysPerWeek;
  }

  int julianDayNumber() const { return m_julianDayNumber; }

 private:
  int m_julianDayNumber;
};

inline bool operator<(Date x, Date y)
{
  return x.julianDayNumber() < y.julianDayNumber();
}

inline bool operator==(Date x, Date y)
{
  return x.julianDayNumber() == y.julianDayNumber();
}

}
#endif  // MUDUO_BASE_DATE_H
