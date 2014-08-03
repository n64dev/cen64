//
// os/windows/timer.c
//
// Functions for mapping ROM images into the address space.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "os/timer.h"
#include <windows.h>

// Computes the difference, in ns, between two times.
unsigned long long compute_time_difference(
  const cen64_time *now, const cen64_time *before) {
  (*now - *before) * 1000000ULL;
}

// Gets the time from the most monotonic source possible.
//
// XXX: Do NOT use QueryPerformanceCounter! It was never
// meant to be a good source of time, and never will be.
void get_time(cen64_time *t) {
  *t = timeGetTime();
}

