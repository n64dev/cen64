//
// os/winapi/timer.c: Functions for obtaining the system clock time.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "os/winapi/timer.h"
#include <windows.h>

// Computes the difference, in ns, between two times.
unsigned long long compute_time_difference(
  const cen64_time *now, const cen64_time *before) {
  return (*now - *before) * 1000000ULL;
}

// Gets the time from the most monotonic source possible.
//
// XXX: Do NOT use QueryPerformanceCounter! It was never
// meant to be a good source of time, and never will be.
void get_time(cen64_time *t) {
  *t = timeGetTime();
}

