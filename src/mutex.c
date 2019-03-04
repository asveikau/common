/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/mutex.h>

#if defined(__APPLE__)
#include <pthread_spis.h>
#endif

#include <assert.h>

void
mutex_init(mutex *m, error *err)
{
#if defined(MUTEX_WINDOWS)
   InitializeCriticalSection(&m->CriticalSection);
#elif defined(MUTEX_PTHREAD)
   int r = 0;
   pthread_mutexattr_t *attr = NULL;
#if defined(__APPLE__)
   pthread_mutexattr_t attrStorage;
   r = pthread_mutexattr_init(&attrStorage);
   if (r)
      ERROR_SET(err, errno, r);
   attr = &attrStorage;
   pthread_mutexattr_setpolicy_np(attr, _PTHREAD_MUTEX_POLICY_FIRSTFIT);
#endif
   r = pthread_mutex_init(&m->m, attr);
   if (r)
      ERROR_SET(err, errno, r);
exit:
   if (attr)
      pthread_mutexattr_destroy(attr);
#else
#error
#endif
}

void
mutex_destroy(mutex *m)
{
#if defined(MUTEX_WINDOWS)
   DeleteCriticalSection(&m->CriticalSection);
#elif defined(MUTEX_PTHREAD)
   pthread_mutex_destroy(&m->m);
#else
#error
#endif
}

void
mutex_acquire(mutex *m)
{
#if defined(MUTEX_WINDOWS)
   EnterCriticalSection(&m->CriticalSection);
#elif defined(MUTEX_PTHREAD)
   int r = pthread_mutex_lock(&m->m);
   assert(!r); // XXX
#else
#error
#endif
}

void
mutex_release(mutex *m)
{
#if defined(MUTEX_WINDOWS)
   LeaveCriticalSection(&m->CriticalSection);
#elif defined(MUTEX_PTHREAD)
   int r = pthread_mutex_unlock(&m->m);
   assert(!r); // XXX
#else
#error
#endif
}

