/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef size_h_
#define size_h_

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

int
size_add(size_t a, size_t b, size_t *sum);

int
size_mult(size_t a, size_t b, size_t *product);

#if defined(__cplusplus)
}
#endif
#endif
