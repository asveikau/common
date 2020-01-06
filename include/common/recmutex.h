/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef recmutex_h
#define recmutex_h

#include "mutex.h"

#if defined(MUTEX_WINDOWS)
typedef DWORD recmutex_owner;
#elif defined(MUTEX_PTHREAD)
typedef pthread_t recmutex_owner;
#endif

struct waiter_node;

typedef struct
{
   mutex lock;
   recmutex_owner owner;
   int acquire_count;
   struct waiter_node *waiters;
} recmutex;

void
recmutex_init(recmutex *, error *);

void
recmutex_destroy(recmutex *);

void
recmutex_acquire(recmutex *);

void
recmutex_release(recmutex *);

#endif

