/*
 Copyright (C) 2019 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/rwlock.h>
#include <assert.h>

#include "rwlock-xpsup.h"

void
rwlock_init(rwlock *lock, error *err)
{
   memset(lock, 0, sizeof(*lock));
#if defined(_WINDOWS)
   lock->lock = ((SRWLOCK)SRWLOCK_INIT);
#if defined(XP_SUPPORT)
   TryLoadPostXpSupport();
   if (!PostXpSupportLoaded())
   {
      lock->using_self = true;
      rwlock_self_init(&lock->self, err);
   }
#endif // XP support
#else // windows
   pthread_rwlockattr_t *attr = NULL;
   int r = 0;

   r = pthread_rwlock_init(&lock->lock, attr);
   if (r)
      ERROR_SET(err, errno, r);
#endif
exit:;
}

void
rwlock_destroy(rwlock *lock)
{
#if defined(LIBCOMMON_RWLOCK_SELF_FALLBACK)
   if (lock->using_self)
   {
      rwlock_self_destroy(&lock->self);
      return;
   }
#endif

#if !defined(_WINDOWS)
   int r = 0;

   r = pthread_rwlock_destroy(&lock->lock);
   assert(!r); // XXX
#endif
}

void
rwlock_acquire_exclusive(rwlock *lock)
{
#if defined(LIBCOMMON_RWLOCK_SELF_FALLBACK)
   if (lock->using_self)
   {
      rwlock_self_acquire_exclusive(&lock->self);
      return;
   }
#endif

#if defined(_WINDOWS)
   AcquireSRWLockExclusive(&lock->lock);
#else
   int r = 0;

   r = pthread_rwlock_wrlock(&lock->lock);
   assert(!r); // XXX
#endif
}

void
rwlock_acquire_shared(rwlock *lock)
{
#if defined(LIBCOMMON_RWLOCK_SELF_FALLBACK)
   if (lock->using_self)
   {
      rwlock_self_acquire_shared(&lock->self);
      return;
   }
#endif

#if defined(_WINDOWS)
   AcquireSRWLockShared(&lock->lock);
#else
   int r = 0;

   r = pthread_rwlock_rdlock(&lock->lock);
   assert(!r); // XXX
#endif
}

void
rwlock_release_exclusive(rwlock *lock)
{
#if defined(LIBCOMMON_RWLOCK_SELF_FALLBACK)
   if (lock->using_self)
   {
      rwlock_self_release_exclusive(&lock->self);
      return;
   }
#endif

#if defined(_WINDOWS)
   ReleaseSRWLockExclusive(&lock->lock);
#else
   int r = 0;

   r = pthread_rwlock_unlock(&lock->lock);
   assert(!r); // XXX
#endif
}

void
rwlock_release_shared(rwlock *lock)
{
#if defined(LIBCOMMON_RWLOCK_SELF_FALLBACK)
   if (lock->using_self)
   {
      rwlock_self_release_shared(&lock->self);
      return;
   }
#endif

#if defined(_WINDOWS)
   ReleaseSRWLockShared(&lock->lock);
#else
   int r = 0;

   r = pthread_rwlock_unlock(&lock->lock);
   assert(!r); // XXX
#endif
}
