//
// os/winapi/thread.h: Multi-threading functions and types.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
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
typedef CONDITION_VARIABLE cen64_cv;

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
// Returns a pointer to the currently executing thread.
//
static inline int cen64_thread_get_current(cen64_thread *t) {
  *t = GetCurrentThread();
  return 0;
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
// Set the affinity of a thread to the CPU mask given by mask.
// Assumes the host system has <= 32 CPUs, but good enough for now.
//
static inline int cen64_thread_setaffinity(cen64_thread *t, uint32_t mask) {
  DWORD winapi_mask = mask;
  return !SetThreadAffinityMask(*t, &winapi_mask);
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
  // ???
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

//
// Condition variables.
//

// Allocates resources for/initializes a CV.
static inline int cen64_cv_create(cen64_cv *cv) {
  InitializeConditionVariable(cv);
  return 0;
}

// Releases resources acquired by cen64_cv_create.
static inline int cen64_cv_destroy(cen64_cv *cv) {
  // ???
  return 0;
}

// Releases the mutex and waits until cen64_cv_signal is called.
static inline int cen64_cv_wait(cen64_cv *cv, cen64_mutex *m) {
  return !SleepConditionVariableCS(cv, m, INFINITE);
}

// Signals the condition variable.
static inline int cen64_cv_signal(cen64_cv *cv) {
  WakeConditionVariable(cv);
  return 0;
}

#endif

