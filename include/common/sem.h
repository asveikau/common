/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef semaphore_h
#define semaphore_h

#include "mutex.h"

#if defined(__APPLE__)
#define SEMAPHORE_LIBDISPATCH
#endif

#if defined(SEMAPHORE_MACH)
#include <mach/mach_init.h>
#include <mach/task.h>
#include <mach/semaphore.h>
#endif

#if defined(SEMAPHORE_LIBDISPATCH)
#include <dispatch/dispatch.h>
#endif

#if defined(MUTEX_PTHREAD)
#include <semaphore.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct
{
#if defined(MUTEX_WINDOWS)
   HANDLE Semaphore;
#elif defined(SEMAPHORE_MACH)
   semaphore_t sem;
#elif defined(SEMAPHORE_LIBDISPATCH)
   dispatch_semaphore_t sem;
#elif defined(MUTEX_PTHREAD)
   sem_t sem;
#else
#error
#endif
} semaphore;

void
sm_init(semaphore *, int, error *);

void
sm_destroy(semaphore *);

void
sm_wait(semaphore *);

void
sm_post(semaphore *);

#if defined(__cplusplus)
}
#endif

#endif
