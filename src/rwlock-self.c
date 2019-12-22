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

#include <common/cas.h>
#include <common/mutex.h>
#include <common/rwlock-self.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WINDOWS)
#include <windows.h>
#else
#include <unistd.h>
#include <errno.h>
#endif

#if defined(__linux__)
#include <sys/eventfd.h>
#endif

// Represents and blocking thread.
// This structure will exist on the stack of each thread to obviate
// the need for memory allocation.  But creating kernel events to
// block can fail, in which case we will spin.
//
struct rwlock_self_waiter_node
{
#if defined(_WINDOWS)
   HANDLE event;
#else
   int pipe[2];
#endif
   volatile bool wakeup;
   struct rwlock_self_waiter_node *next;
};

//
// OS specific blocking primitives.
//

static void
waiter_node_init(struct rwlock_self_waiter_node *node)
{
   memset(node, 0, sizeof(*node));
#if defined(_WINDOWS)
   node->event = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
#if defined(__linux__)
   node->pipe[0] = node->pipe[1] = eventfd(0, EFD_CLOEXEC);
   if (node->pipe[0] >= 0)
      return;
#endif
   if (pipe(node->pipe))
      node->pipe[0] = node->pipe[1] = -1;
#endif
}

static void
waiter_node_destroy(struct rwlock_self_waiter_node *node)
{
#if defined(_WINDOWS)
   if (node->event)
   {
      CloseHandle(node->event);
      node->event = NULL;
   }
#else
   if (node->pipe[0] >= 0)
      close(node->pipe[0]);
   if (node->pipe[1] >= 0 && node->pipe[0] != node->pipe[1])
      close(node->pipe[1]);
   node->pipe[0] = node->pipe[1] = -1;
#endif
}

static void
waiter_node_wait(struct rwlock_self_waiter_node *node)
{
#if defined(_WINDOWS)
   if (node->event)
   {
      WaitForSingleObject(node->event, INFINITE);
      return;
   }
#else
   if (node->pipe[0] >= 0)
   {
      int64_t i;
      int r = 0;
      do
      {
         r = read(node->pipe[0], &i, sizeof(i));
      } while (r < 0 && (errno == EINTR || errno == EAGAIN));
      if (r < sizeof(i))
         abort();
      return;
   }
#endif
   while (!node->wakeup)
      ;
}

static void
waiter_node_signal(struct rwlock_self_waiter_node *node)
{
#if defined(_WINDOWS)
   if (node->event)
   {
      SetEvent(node->event);
      return;
   }
#else
   if (node->pipe[1] >= 0)
   {
      int64_t i = 1;
      int r = 0;
      do
      {
         r = read(node->pipe[1], &i, sizeof(i));
      } while (r < 0 && (errno == EINTR || errno == EAGAIN));
      if (r < sizeof(i))
         abort();
      return;
   }
#endif
   node->wakeup = true;
   memory_barrier();
}

//
// Enqueue into waiters list
//

static void
waiter_node_enqueue(
   struct rwlock_self_waiter_node **head,
   struct rwlock_self_waiter_node *node
)
{
   node->next = *head;
   *head = node;
}

static void
rwlock_self_wakeup(struct rwlock_self *lock)
{
   struct rwlock_self_waiter_node *node = NULL;
   int *inc = NULL;

   if (!lock->num_writers && !lock->num_writers && lock->writers)
   {
      // Wake up exactly one writer.
      //
      node = lock->writers;
      lock->writers = node->next;
      node->next = NULL;
      inc = &lock->num_writers;
   }
   else if (lock->readers)
   {
      // Wake up all readers.
      //
      node = lock->readers;
      lock->readers = NULL;
      inc = &lock->num_readers;
   }

   while (node)
   {
      struct rwlock_self_waiter_node *next = node->next;
      waiter_node_signal(node);
      node = next;
      ++*inc;
   }
}

void
rwlock_self_init(struct rwlock_self *lock, error *err)
{
   memset(lock, 0, sizeof(*lock));
   mutex_init(&lock->lock, err);
}

void
rwlock_self_destroy(struct rwlock_self *lock)
{
   mutex_destroy(&lock->lock);
}

void
rwlock_self_acquire_exclusive(struct rwlock_self *lock)
{
   bool block = false;
   struct rwlock_self_waiter_node self = {0};

   mutex_acquire(&lock->lock);
   if (lock->num_readers || lock->num_writers || lock->writers)
   {
      block = true;
      waiter_node_init(&self);
      waiter_node_enqueue(&lock->writers, &self);
   }
   else
   {
      ++lock->num_writers;
   }
   mutex_release(&lock->lock);

   if (block)
   {
      waiter_node_wait(&self);
      waiter_node_destroy(&self);
   }
}

void
rwlock_self_release_exclusive(struct rwlock_self *lock)
{
   mutex_acquire(&lock->lock);
   --lock->num_writers;
   rwlock_self_wakeup(lock);
   mutex_release(&lock->lock);
}

void
rwlock_self_acquire_shared(struct rwlock_self *lock)
{
   bool block = false;
   struct rwlock_self_waiter_node self = {0};

   mutex_acquire(&lock->lock);
   if (lock->num_writers)
   {
      block = true;
      waiter_node_init(&self);
      waiter_node_enqueue(&lock->readers, &self);
   }
   else
   {
      ++lock->num_readers;
   }
   mutex_release(&lock->lock);

   if (block)
   {
      waiter_node_wait(&self);
      waiter_node_destroy(&self);
   }
}

void
rwlock_self_release_shared(struct rwlock_self *lock)
{
   mutex_acquire(&lock->lock);
   --lock->num_readers;
   rwlock_self_wakeup(lock);
   mutex_release(&lock->lock);
}