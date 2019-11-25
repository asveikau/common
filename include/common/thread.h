/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_thread_h
#define common_thread_h

#include "mutex.h"
#include "error.h"
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(MUTEX_WINDOWS)
#define THREAD_PROC_RETVAL DWORD WINAPI
typedef DWORD (WINAPI *thread_proc)(void *context);
#elif defined(MUTEX_PTHREAD)
#define THREAD_PROC_RETVAL void *
typedef void *(*thread_proc)(void *context);
#else
#error port me
#endif

typedef struct
{
#if defined(MUTEX_WINDOWS)
   DWORD Id;
   HANDLE Handle;
#elif defined(MUTEX_PTHREAD)
   pthread_t th;
   bool init;
#else
#error port me
#endif
} thread_id;

void
create_thread(
   void *context,
   thread_proc fn,
   thread_id *id_out,
   error *err
);

void
detach_thread(thread_id *id);

void
join_thread(thread_id *id);

bool
thread_is_self(thread_id *id);

void
thread_get_self(thread_id *id);

bool
thread_is_started(thread_id *id);

#if defined(__cplusplus)
}

#include <functional>

namespace common
{
   void
   create_thread(
      const std::function<void(void)> &fn,
      thread_id *id_out,
      error *err
   );
}

#endif
#endif
