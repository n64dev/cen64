//
// os/unix/rom_file.c
//
// Functions for mapping ROM images into the address space.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "local_time.h"
#include <time.h>

void get_local_time(struct time_stamp *ts) {
  time_t now = time(NULL);
  struct tm time = { 0, };
  localtime_r(&now, &time);

  ts->year  = time.tm_year;
  ts->month = time.tm_mon + 1; // month is zero-indexed in this struct
  ts->day   = time.tm_mday;
  ts->hour  = time.tm_hour;
  ts->min   = time.tm_min;
  ts->sec   = time.tm_sec;
  ts->week_day = time.tm_wday;
}
