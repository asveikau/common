/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef crypto_md5_h
#define crypto_md5_h

#include "hash.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct md5_state
{
   struct crypto_hash base;
   uint32_t h[4];
};

void
md5_init(
   struct md5_state *state
);

void
md5_hash_data(
   struct md5_state *state,
   const void *buf,
   size_t len
);

void
md5_final(
   struct md5_state *state,
   void **buffer,
   size_t *len
);

#if defined(__cplusplus)
}

namespace common
{
   typedef hash_base<md5_state, md5_init, md5_hash_data, md5_final>
   md5_hasher;
}
#endif
#endif
