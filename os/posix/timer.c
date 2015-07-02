//
// os/posix/timer.c: Functions for obtaining the system clock time.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "os/posix/timer.h"
#include <time.h>
#include <sys/time.h>

// Computes the difference, in ns, between two times.
unsigned long long compute_time_difference(
  const cen64_time *now, const cen64_time *before) {
#if defined(__APPLE__)
  return (now->tv_sec - before->tv_sec) * NS_PER_SEC +
    (now->tv_usec - before->tv_usec) * NS_PER_USEC;
#else
  return (now->tv_sec - before->tv_sec) * NS_PER_SEC +
    (now->tv_nsec - before->tv_nsec);
#endif
}

// Gets the time from the most monotonic source possible.
void get_time(cen64_time *t) {
#if defined(__APPLE__)
  gettimeofday(t, NULL);
#else
  clock_gettime(GETTIME_SOURCE, t);
#endif
}

