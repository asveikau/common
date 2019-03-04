/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/size.h>
#include <limits.h>

#ifndef SIZE_MAX
#define SIZE_MAX (~(size_t)0)
#endif

int
size_add(size_t a, size_t b, size_t *sum)
{
   int r = SIZE_MAX - a < b;
   if (!r)
      *sum = a + b;
   return r;
}

int
size_mult(size_t a, size_t b, size_t *product)
{
   int r = a && (SIZE_MAX/a < b);
   if (!r)
      *product = a*b;
   return r;
}
