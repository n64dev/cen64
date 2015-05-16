//
// os/winapi/timer.h: Functions for obtaining the system clock time.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_WINAPI_TIMER
#define CEN64_OS_WINAPI_TIMER
#include "common.h"
#include <windows.h>

#define NS_PER_SEC 1000000000ULL

typedef DWORD cen64_time;

cen64_cold unsigned long long compute_time_difference(
  const cen64_time *now, const cen64_time *before);

cen64_cold void get_time(cen64_time *t);

#endif

