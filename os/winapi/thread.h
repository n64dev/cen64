//
// os/winapi/thread.h: Multi-threading functions and types.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_WINAPI_THREAD
#define CEN64_OS_WINAPI_THREAD
#include "common.h"
#include <windows.h>

#define CEN64_THREAD_RETURN_TYPE DWORD
#define CEN64_THREAD_RETURN_VAL 0

// Type definitions.
typedef HANDLE cen64_thread;
typedef DWORD (*cen64_thread_func)(LPVOID arg);

typedef CRITICAL_SECTION cen64_mutex;

//
// Threads.
//

// Creates a thread and starts executing 'f' within the thread.
static inline int cen64_thread_create(cen64_thread *t,
  cen64_thread_func f, void *arg) {
  if (likely((*t = CreateThread(NULL, 0, f, arg, 0, NULL)) != NULL))
    return 0;

  return 1;
}

//
// Join a thread created with cen64_thread_create. Use this to
// effectively "free" the resources acquired for the thread.
//
static inline int cen64_thread_join(cen64_thread *t) {
  if (unlikely(WaitForSingleObject(*t, INFINITE) != WAIT_OBJECT_0))
    return 1;

  return !CloseHandle(*t);
}

//
// Mutexes.
//

// Allocates resources for/initializes a mutex.
static inline int cen64_mutex_create(cen64_mutex *m) {
  InitializeCriticalSection(m);
  return 0;
}

// Releases resources acquired by cen64_mutex_create.
static inline int cen64_mutex_destroy(cen64_mutex *m) {
  DeleteCriticalSection(m);
  return 0;
}

// Locks the mutex passed as an argument.
static inline int cen64_mutex_lock(cen64_mutex *m) {
  EnterCriticalSection(m);
  return 0;
}

// Unlocks the mutex passed as an argument.
static inline int cen64_mutex_unlock(cen64_mutex *m) {
  LeaveCriticalSection(m);
  return 0;
}

#endif

