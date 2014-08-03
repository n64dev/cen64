//
// os/unix/timer.c
//
// Functions for mapping ROM images into the address space.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "os/timer.h"
#include <time.h>

// Computes the difference, in ns, between two times.
unsigned long long compute_time_difference(
  const cen64_time *now, const cen64_time *before) {
  return (now->tv_sec - before->tv_sec) * NS_PER_SEC +
    (now->tv_nsec - before->tv_nsec);
}

// Gets the time from the most monotonic source possible.
void get_time(cen64_time *t) {
#ifdef __linux__
  clock_gettime(CLOCK_MONOTONIC_RAW, t);
#elif defined(_POSIX_MONOTONIC_CLOCK)
  clock_gettime(CLOCK_MONOTONIC, t);
#else
  clock_gettime(CLOCK_REALTIME, t);
#endif
}

