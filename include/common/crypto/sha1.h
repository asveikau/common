/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef crypto_sha1_h
#define crypto_sha1_h

#include "hash.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct sha1_state
{
   struct crypto_hash base;
   uint32_t buffer[80];
   uint32_t h[5];
};

void
sha1_init(
   struct sha1_state *state
);

void
sha1_hash_data(
   struct sha1_state *state,
   const void *buf,
   size_t len
);

void
sha1_final(
   struct sha1_state *state,
   void **buffer,
   size_t *len
);

#if defined(__cplusplus)
}

namespace common
{
   typedef hash_base<sha1_state, sha1_init, sha1_hash_data, sha1_final>
   sha1_hasher;
}

#endif
#endif
