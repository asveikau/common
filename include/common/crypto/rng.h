/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef crypto_rng_h_
#define crypto_rng_h_

#include "../error.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct rng_state;

void
rng_init(
   struct rng_state **state,
   error *err
);

void
rng_generate(
   struct rng_state *rng,
   void *buffer,
   size_t len,
   error *err
);

void
rng_close(struct rng_state *rng);

#if defined(__cplusplus)
}
#endif
#endif
