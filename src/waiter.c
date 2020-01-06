/*
 Copyright (C) 2018-2020 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/waiter.h>
#include <common/cas.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef _WINDOWS
#include <unistd.h>
#include <errno.h>
#endif

#if defined(__linux__)
#include <sys/eventfd.h>
#endif

void
waiter_node_init(struct waiter_node *node)
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

void
waiter_node_destroy(struct waiter_node *node)
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

void
waiter_node_wait(struct waiter_node *node)
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

void
waiter_node_signal(struct waiter_node *node)
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
         r = write(node->pipe[1], &i, sizeof(i));
      } while (r < 0 && (errno == EINTR || errno == EAGAIN));
      if (r < sizeof(i))
         abort();
      return;
   }
#endif
   node->wakeup = true;
   memory_barrier();
}
