/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef lazy_h
#define lazy_h

#include "error.h"

#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct
{
   volatile unsigned long state;
} lazy_init_state;

void
lazy_init(
  lazy_init_state *state,
  void (*fn)(void *context, error *err),
  void *context,
  error *err
);

bool
lazy_is_initialized(lazy_init_state *state);

#if defined(__cplusplus)
}
#endif
#endif
