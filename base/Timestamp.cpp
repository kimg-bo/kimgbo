#include <sys/time.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cassert>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS
#include "Timestamp.h"

using namespace kimgbo;

static_assert(sizeof(Timestamp) == sizeof(int64_t), "sizeof(Timestamp) != sizeof(int64_t)");

Timestamp::Timestamp(int64_t microSecondsSinceEpoch):m_microSecondsSinceEpoch(microSecondsSinceEpoch)
{
}

std::string Timestamp::toString() const
{
  char buf[32] = {0};
  int64_t seconds = m_microSecondsSinceEpoch / kMicroSecondsPerSecond;
  int64_t microseconds = m_microSecondsSinceEpoch % kMicroSecondsPerSecond;
  snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
  
  return buf;
}

std::string Timestamp::toFormattedString() const
{
	char buf[32];
	time_t seconds = static_cast<time_t>(m_microSecondsSinceEpoch / kMicroSecondsPerSecond);
  int microseconds = static_cast<int>(m_microSecondsSinceEpoch % kMicroSecondsPerSecond);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);

  snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d", tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour, 
  					tm_time.tm_min, tm_time.tm_sec, microseconds);
  return buf;
}

Timestamp Timestamp::now()
{
	struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}
	
Timestamp Timestamp::invalid()
{
	return Timestamp();
}