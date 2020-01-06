/*
 Copyright (C) 2018-2020 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_waiter_h__
#define common_waiter_h__

#if defined(_WINDOWS)
#include <windows.h>
#endif

#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct waiter_node 
{
   // Since this structure is most interesting in a queue.
   //
   struct waiter_node *next;

   // First attempt will be to use a proper kernel wait primitive.
   //
#if defined(_WINDOWS)
   HANDLE event;
#else
   int pipe[2];
#endif

   // If that fails due to handle/fd exhaustion or something, we'll
   // spin.
   //
   volatile bool wakeup;
};

void
waiter_node_init(struct waiter_node *node);

void
waiter_node_destroy(struct waiter_node *node);

void
waiter_node_signal(struct waiter_node *node);

void
waiter_node_wait(struct waiter_node *node);

#if defined(__cplusplus)
}
#endif
#endif
