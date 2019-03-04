/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef mutex_h
#define mutex_h

#include "error.h"

#if defined(_WINDOWS)
#include <windows.h>
#define MUTEX_WINDOWS
#else
#include <pthread.h>
#define MUTEX_PTHREAD
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct
{
#if defined(MUTEX_WINDOWS)
   CRITICAL_SECTION CriticalSection;
#elif defined(MUTEX_PTHREAD)
   pthread_mutex_t m;
#endif
} mutex;

void
mutex_init(mutex *, error *err);

void
mutex_destroy(mutex *);

void
mutex_acquire(mutex *);

void
mutex_release(mutex *);

#if defined(__cplusplus)
}
#endif

#endif
