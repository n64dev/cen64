//
// os/winapi/local_time.c: Time functions for Windows.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "local_time.h"
#include <time.h>
#include <windows.h>

void get_local_time(struct time_stamp *ts) {
  SYSTEMTIME sysTime;
  GetLocalTime(&sysTime);

  ts->year  = sysTime.wYear;
  ts->month = sysTime.wMonth;
  ts->day   = sysTime.wDay;
  ts->hour  = sysTime.wHour;
  ts->min   = sysTime.wMinute;
  ts->sec   = sysTime.wSecond;
  ts->week_day = sysTime.wDayOfWeek - 1;
}
