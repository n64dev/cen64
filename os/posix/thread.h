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
#include <pthread.h>

#define CEN64_THREAD_RETURN_TYPE void*
#define CEN64_THREAD_RETURN_VAL NULL

// Type definitions.
typedef pthread_t cen64_thread;
typedef void* (*cen64_thread_func)(void *arg);

typedef pthread_mutex_t cen64_mutex;

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

#endif

