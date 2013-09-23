/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2013, The Tor Project, Inc. */
/* See LICENSE for licensing information */

#ifndef TOR_COMPAT_THREADS_H
#define TOR_COMPAT_THREADS_H

#include "orconfig.h"
#include "testsupport.h"
#include "compat.h"

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#define WIN32_LEAN_AND_MEAN
#endif

#if defined(HAVE_PTHREAD_H) && !defined(_WIN32)
#include <pthread.h>
#endif

int spawn_func(void (*func)(void *), void *data);
void spawn_exit(void) ATTR_NORETURN;

#if defined(ENABLE_THREADS) && defined(_WIN32)
#define USE_WIN32_THREADS
#define TOR_IS_MULTITHREADED 1
#elif (defined(ENABLE_THREADS) && defined(HAVE_PTHREAD_H) && \
       defined(HAVE_PTHREAD_CREATE))
#define USE_PTHREADS
#define TOR_IS_MULTITHREADED 1
#else
#undef TOR_IS_MULTITHREADED
#endif

/* Because we use threads instead of processes on most platforms (Windows,
 * Linux, etc), we need locking for them.  On platforms with poor thread
 * support or broken gethostbyname_r, these functions are no-ops. */

/** A generic lock structure for multithreaded builds. */
typedef struct tor_mutex_t {
#if defined(USE_WIN32_THREADS)
  /** Windows-only: on windows, we implement locks with CRITICAL_SECTIONS. */
  CRITICAL_SECTION mutex;
#elif defined(USE_PTHREADS)
  /** Pthreads-only: with pthreads, we implement locks with
   * pthread_mutex_t. */
  pthread_mutex_t mutex;
#else
  /** No-threads only: Dummy variable so that tor_mutex_t takes up space. */
  int _unused;
#endif
} tor_mutex_t;

#ifdef TOR_IS_MULTITHREADED
tor_mutex_t *tor_mutex_new(void);
void tor_mutex_init(tor_mutex_t *m);
void tor_mutex_acquire(tor_mutex_t *m);
void tor_mutex_release(tor_mutex_t *m);
void tor_mutex_free(tor_mutex_t *m);
void tor_mutex_uninit(tor_mutex_t *m);
unsigned long tor_get_thread_id(void);
void tor_threads_init(void);
#else
#define tor_mutex_new() ((tor_mutex_t*)tor_malloc(sizeof(int)))
#define tor_mutex_init(m) STMT_NIL
#define tor_mutex_acquire(m) STMT_VOID(m)
#define tor_mutex_release(m) STMT_NIL
#define tor_mutex_free(m) STMT_BEGIN tor_free(m); STMT_END
#define tor_mutex_uninit(m) STMT_NIL
#define tor_get_thread_id() (1UL)
#define tor_threads_init() STMT_NIL
#endif

void set_main_thread(void);
int in_main_thread(void);

#ifdef TOR_IS_MULTITHREADED
typedef struct tor_cond_t {
#ifdef USE_PTHREADS
  pthread_cond_t cond;
#elif defined(USE_WIN32_THREADS)
  HANDLE event;

  CRITICAL_SECTION lock;
  int n_waiting;
  int n_to_wake;
  int generation;
#else
#error no known condition implementation.
#endif
} tor_cond_t;
tor_cond_t *tor_cond_new(void);
void tor_cond_free(tor_cond_t *cond);
int tor_cond_wait(tor_cond_t *cond, tor_mutex_t *mutex,
                  const struct timeval *tv);
void tor_cond_signal_one(tor_cond_t *cond);
void tor_cond_signal_all(tor_cond_t *cond);
#endif

#endif

