//
// os/timer.h
//
// Functions for mapping ROM images into the address space.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef __os_timer_h__
#define __os_timer_h__
#define NS_PER_SEC 1000000000ULL

#ifdef _WIN32
#include <windows.h>
typedef DWORD cen64_time;

#else
#include <time.h>
typedef struct timespec cen64_time;
#endif

unsigned long long compute_time_difference(
  const cen64_time *now, const cen64_time *before);

void get_time(cen64_time *t);

#endif

