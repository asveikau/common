/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/recmutex.h>
#include <common/sem.h>
#include <assert.h>

#include <stdlib.h>
#include <string.h>

struct recmutex_waiter
{
   struct recmutex_waiter *next;

#if defined(MUTEX_WINDOWS)
   HANDLE EventHandle;
#else
   semaphore sem;
#endif
};

static void
recmutex_wait_init(struct recmutex_waiter *self, error *err)
{
#if defined(MUTEX_WINDOWS)
   if (!(self->EventHandle = CreateEvent(NULL, FALSE, FALSE, NULL)))
   {
      ERROR_SET(err, win32, GetLastError());
   }
exit:;
#else
   sm_init(&self->sem, 0, err);
#endif
}

static void
recmutex_wait(struct recmutex_waiter *self)
{
#if defined(MUTEX_WINDOWS)
   WaitForSingleObject(self->EventHandle, INFINITE);
   CloseHandle(self->EventHandle);
#else
   sm_wait(&self->sem);
   sm_destroy(&self->sem);
#endif
}

static void
recmutex_signal(struct recmutex_waiter *other)
{
#if defined(MUTEX_WINDOWS)
   SetEvent(other->EventHandle);
#else
   sm_post(&other->sem);
#endif
}

static int
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
   struct recmutex_waiter self;
   int wait = 0;

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
      error err = {0};
      recmutex_wait_init(&self, &err);
      if (ERROR_FAILED(&err))
         abort(); // TODO: clean this up

      self.next = m->waiters;
      m->waiters = &self;
      wait = 1;
   }
   mutex_release(&m->lock);

   if (wait)
   {
      recmutex_wait(&self);
      wait = 0;
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
      struct recmutex_waiter *waiter = m->waiters;
      if (waiter)
      {
         m->waiters = waiter->next;
         recmutex_signal(waiter);
      }
   }

   mutex_release(&m->lock);
}
