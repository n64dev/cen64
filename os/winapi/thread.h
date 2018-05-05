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
#include <errno.h>
#include <windows.h>

#define CEN64_THREAD_RETURN_TYPE DWORD
#define CEN64_THREAD_RETURN_VAL 0

// Type definitions.
typedef HANDLE cen64_thread;
typedef DWORD (*cen64_thread_func)(LPVOID arg);

typedef HANDLE cen64_mutex;
typedef HANDLE cen64_cv;

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

// Sets the name of the thread to a specific value
// This function is API dependent and must be called either
// before starting the thread or with older Windows APIs directly after the creation.
// If you call it at the wrong time or your OS doesn't support custom thread names
// the return value will be non-zero.
static inline int cen64_thread_setname(cen64_thread *t, const char *name) {
  #ifdef HAVE_WINDOWS_GETTHREADID
    SetThreadName(GetThreadId(t), name);
    return 0;
  #elif HAVE_WINDOWS_GETCURRENTTHREADID
    SetThreadName(GetCurrentThreadId(), name);
    return 0;
  #endif

  return ENOSYS;
}

//
// Mutexes.
//

// Allocates resources for/initializes a mutex.
static inline int cen64_mutex_create(cen64_mutex *m) {
  if (unlikely((*m = CreateMutex(NULL, FALSE, NULL)) == NULL))
    return 1;

  return 0;
}

// Releases resources acquired by cen64_mutex_create.
static inline int cen64_mutex_destroy(cen64_mutex *m) {
  return !CloseHandle(*m);
}

// Locks the mutex passed as an argument.
static inline int cen64_mutex_lock(cen64_mutex *m) {
  if (likely(WaitForSingleObject(*m, INFINITE) == WAIT_OBJECT_0))
    return 0;

  return 1;
}

// Unlocks the mutex passed as an argument.
static inline int cen64_mutex_unlock(cen64_mutex *m) {
  return !ReleaseMutex(*m);
}

//
// Condition variables.
//

// Allocates resources for/initializes a CV.
static inline int cen64_cv_create(cen64_cv *cv) {
  if (unlikely((*cv = CreateSemaphore(NULL, 0, 1, NULL)) == NULL))
    return 1;

  return 0;
}

// Releases resources acquired by cen64_cv_create.
static inline int cen64_cv_destroy(cen64_cv *cv) {
  return !CloseHandle(*cv);
}

// Releases the mutex and waits until cen64_cv_signal is called.
static inline int cen64_cv_wait(cen64_cv *cv, cen64_mutex *m) {
  if (likely(SignalObjectAndWait(*m, *cv, INFINITE, FALSE) == WAIT_OBJECT_0))
    return 0;

  return 1;
}

// Signals the condition variable.
static inline int cen64_cv_signal(cen64_cv *cv) {
  return !ReleaseSemaphore(*cv, 1, 0);
}

#endif

