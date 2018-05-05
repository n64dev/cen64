//
// os/posix/thread.h: Multi-threading functions and types.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2015, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef CEN64_OS_POSIX_THREAD
#define CEN64_OS_POSIX_THREAD
#include "common.h"
#include <errno.h>
#include <pthread.h>

#define CEN64_THREAD_RETURN_TYPE void*
#define CEN64_THREAD_RETURN_VAL NULL

// Type definitions.
typedef pthread_t cen64_thread;
typedef void* (*cen64_thread_func)(void *arg);

typedef pthread_mutex_t cen64_mutex;
typedef pthread_cond_t cen64_cv;

//
// Threads.
//

// Creates a thread and starts executing 'f' within the thread.
static inline int cen64_thread_create(cen64_thread *t,
  cen64_thread_func f, void *arg) {
  return pthread_create(t, NULL, f, arg);
}

//
// Join a thread created with cen64_thread_create. Use this to
// effectively "free" the resources acquired for the thread.
//
static inline int cen64_thread_join(cen64_thread *t) {
  return pthread_join(*t, NULL);
}

// Sets the name of the thread to a specific value
// This function is os dependent and must be called either
// before starting the thread or on macOS directly after the creation.
// If you call it at the wrong time or your OS doesn't support custom thread names
// the return value will be non-zero.
static inline int cen64_thread_setname(cen64_thread *t, const char *name) {
  #ifdef __APPLE__
    if (t == NULL)
      return pthread_setname_np(name);
  #elif __NETBSD__
    if (t != NULL)
      return pthread_setname_np(t, "%s", name);
  #else
    if (t != NULL)
      return pthread_setname_np(t, name);
  #endif

  return ENOSYS;
}

//
// Mutexes.
//

// Allocates resources for/initializes a mutex.
static inline int cen64_mutex_create(cen64_mutex *m) {
  return pthread_mutex_init(m, NULL);
}

// Releases resources acquired by cen64_mutex_create.
static inline int cen64_mutex_destroy(cen64_mutex *m) {
  return pthread_mutex_destroy(m);
}

// Locks the mutex passed as an argument.
static inline int cen64_mutex_lock(cen64_mutex *m) {
  return pthread_mutex_lock(m);
}

// Unlocks the mutex passed as an argument.
static inline int cen64_mutex_unlock(cen64_mutex *m) {
  return pthread_mutex_unlock(m);
}

//
// Condition variables.
//

// Allocates resources for/initializes a CV.
static inline int cen64_cv_create(cen64_cv *cv) {
  return pthread_cond_init(cv, NULL);
}

// Releases resources acquired by cen64_cv_create.
static inline int cen64_cv_destroy(cen64_cv *cv) {
  return pthread_cond_destroy(cv);
}

// Releases the mutex and waits until cen64_cv_signal is called.
static inline int cen64_cv_wait(cen64_cv *cv, cen64_mutex *m) {
  return pthread_cond_wait(cv, m) || pthread_mutex_unlock(m);
}

// Signals the condition variable.
static inline int cen64_cv_signal(cen64_cv *cv) {
  return pthread_cond_signal(cv);
}

#endif

