/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef crypto_sha256_h
#define crypto_sha256_h

#include "hash.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct sha256_state
{
   struct crypto_hash base;
   uint32_t buffer[64];
   uint32_t h[8];
};

void
sha256_init(
   struct sha256_state *state
);

void
sha256_hash_data(
   struct sha256_state *state,
   const void *buf,
   size_t len
);

void
sha256_final(
   struct sha256_state *state,
   void **buffer,
   size_t *len
);

#if defined(__cplusplus)
}

namespace common
{
   typedef hash_base<sha256_state, sha256_init, sha256_hash_data, sha256_final>
   sha256_hasher;
}

#endif
#endif
