/*
 Copyright (C) Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef trie_h
#define trie_h

#include <stddef.h>

#include "error.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct trie
{
   void *value;
   void (*dtor)(void*);
   struct trie *subtries[256];
};

void
trie_insert(
   struct trie **t,
   const void *key,
   size_t keylen,
   void *value,
   void (*dtor)(void*),
   error *err
);

void *
trie_find(
   struct trie *t,
   const void *key,
   size_t keylen
);

size_t
trie_get_prefix_length(
   struct trie *t,
   const void *keyp,
   size_t keylen
);

void
trie_remove(
   struct trie **t,
   const void *key,
   size_t keylen
);

void
trie_free(
   struct trie *t
);

#if defined(__cplusplus)
}
#endif
#endif
