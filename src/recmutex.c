/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/recmutex.h>
#include <common/waiter.h>
#include <common/sem.h>

#include <stdbool.h>
#include <assert.h>

#include <stdlib.h>
#include <string.h>

static bool
recmutex_is_owner(recmutex *m)
{
#if defined(MUTEX_WINDOWS)
   return (GetCurrentThreadId() == m->owner);
#elif defined(MUTEX_PTHREAD)
   return pthread_equal(pthread_self(), m->owner); 
#else
#error
#endif
}

void
recmutex_init(recmutex *m, error *err)
{
   memset(m, 0, sizeof(*m));
   mutex_init(&m->lock, err);
}

void
recmutex_destroy(recmutex *m)
{
   assert(!m->acquire_count);
   assert(!m->waiters);
   mutex_destroy(&m->lock);
}

void
recmutex_acquire(recmutex *m)
{
   struct waiter_node self;
   bool wait = false;

entry:
   mutex_acquire(&m->lock);
   if (!m->acquire_count)
   {
#if defined(MUTEX_WINDOWS)
      m->owner = GetCurrentThreadId();
#elif defined(MUTEX_PTHREAD)
      m->owner = pthread_self();
#else
#error
#endif
      m->acquire_count = 1;
   }
   else if (recmutex_is_owner(m))
   {
      ++m->acquire_count;
   }
   else
   {
      waiter_node_init(&self);
      self.next = m->waiters;
      m->waiters = &self;
      wait = true;
   }
   mutex_release(&m->lock);

   if (wait)
   {
      waiter_node_wait(&self);
      waiter_node_destroy(&self);
      wait = false;
      goto entry;
   }
}

void
recmutex_release(recmutex *m)
{
   mutex_acquire(&m->lock);

   assert(m->acquire_count);
   assert(recmutex_is_owner(m));

   if (!--m->acquire_count)
   {
      struct waiter_node *waiter = m->waiters;
      if (waiter)
      {
         m->waiters = waiter->next;
         waiter_node_signal(waiter);
      }
   }

   mutex_release(&m->lock);
}
