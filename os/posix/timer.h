//
// os/posix/timer.h: Functions for obtaining the system clock time.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_POSIX_TIMER
#define CEN64_OS_POSIX_TIMER
#include "common.h"

#define NS_PER_SEC 1000000000ULL

#if defined(CLOCK_MONOTONIC_PRECISE)
#define GETTIME_SOURCE CLOCK_MONOTONIC_PRECISE
#else
#define GETTIME_SOURCE CLOCK_MONOTONIC_RAW
#endif

#ifdef __APPLE__
#include <time.h>
typedef struct timeval cen64_time;

#else
#include <time.h>
typedef struct timespec cen64_time;
#endif

cen64_cold unsigned long long compute_time_difference(
  const cen64_time *now, const cen64_time *before);

cen64_cold void get_time(cen64_time *t);

#endif

