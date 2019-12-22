/*
 Copyright (C) 2019 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_rwlock_h_
#define common_rwlock_h_

#include <stdbool.h>

#include "error.h"

#if defined(_WINDOWS) && defined(XP_SUPPORT)
#include "rwlock-self.h"
#define LIBCOMMON_RWLOCK_SELF_FALLBACK
#endif

#if !defined(_WINDOWS)
#include <pthread.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct
{
#if defined(_WINDOWS)
   SRWLOCK lock;
#else
   pthread_rwlock_t lock;
#endif
#if defined(LIBCOMMON_RWLOCK_SELF_FALLBACK)
   struct rwlock_self self;
   bool using_self;
#endif
} rwlock;

void
rwlock_init(rwlock *lock, error *err);

void
rwlock_destroy(rwlock *lock);

void
rwlock_acquire_exclusive(rwlock *lock);

void
rwlock_acquire_shared(rwlock *lock);

void
rwlock_release_exclusive(rwlock *lock);

void
rwlock_release_shared(rwlock *lock);

#if defined(__cplusplus)
}
#endif
#endif
