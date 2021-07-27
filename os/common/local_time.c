//
// os/common/local_time.c
//
// Functions for getting the current time.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <time.h>
#include "local_time.h"

void get_local_time(struct time_stamp *ts, int32_t offset_seconds) {
  time_t now = time(NULL);
  now += offset_seconds;

  struct tm tm = { 0, };
#ifdef _WIN32
  localtime_s(&now, &tm);
#else
  localtime_r(&now, &tm);
#endif

  // Copy tm into time_stamp struct
  ts->year     = tm.tm_year;
  ts->month    = tm.tm_mon + 1; // time_stamp month is zero-indexed
  ts->day      = tm.tm_mday;
  ts->hour     = tm.tm_hour;
  ts->min      = tm.tm_min;
  ts->sec      = tm.tm_sec;
  ts->week_day = tm.tm_wday;
}

int32_t get_offset_seconds(const struct time_stamp * ts) {
  struct tm tm = { 0, };

  // Copy time_stamp into tm struct
  tm.tm_year  = ts->year;
  tm.tm_mon   = ts->month - 1; // time_stamp month is zero-indexed
  tm.tm_mday  = ts->day;
  tm.tm_hour  = ts->hour;
  tm.tm_min   = ts->min;
  tm.tm_sec   = ts->sec;
  tm.tm_wday  = ts->week_day;
  tm.tm_isdst = -1; // Auto-adjust for DST

  time_t then = mktime(&tm);
  time_t now = time(NULL);

  return then - now;
}
