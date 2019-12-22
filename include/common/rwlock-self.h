/*
 Copyright (C) 2019 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

//
// This file implements a somewhat naive, unfair rwlock.
//
// It will mostly be used in places where other implementations
// are unavailable.  (eg: Windows XP)
//

#ifndef common_rwlock_self_h_
#define common_rwlock_self_h_

#include "error.h"
#include "mutex.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct rwlock_self_waiter_node;
struct rwlock_self
{
   mutex lock;
   int num_readers;
   int num_writers;
   struct rwlock_self_waiter_node *readers;
   struct rwlock_self_waiter_node *writers;
};

void
rwlock_self_init(struct rwlock_self *lock, error *err);

void
rwlock_self_destroy(struct rwlock_self *lock);

void
rwlock_self_acquire_exclusive(struct rwlock_self *lock);

void
rwlock_self_acquire_shared(struct rwlock_self *lock);

void
rwlock_self_release_exclusive(struct rwlock_self *lock);

void
rwlock_self_release_shared(struct rwlock_self *lock);

#if defined(__cplusplus)
}
#endif
#endif