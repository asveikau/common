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

#include <common/mutex.h>
#include <common/rwlock-self.h>
#include <common/waiter.h>

#include <string.h>

//
// Enqueue into waiters list
//

static void
waiter_node_enqueue(
   struct waiter_node **head,
   struct waiter_node *node
)
{
   node->next = *head;
   *head = node;
}

static void
rwlock_self_wakeup(struct rwlock_self *lock)
{
   struct waiter_node *node = NULL;
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
      struct waiter_node *next = node->next;
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
   struct waiter_node self = {0};

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
   struct waiter_node self = {0};

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