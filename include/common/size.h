/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef size_h_
#define size_h_

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#if defined(__cplusplus)
extern "C" {
#endif

int
size_add(size_t a, size_t b, size_t *sum);

int
size_mult(size_t a, size_t b, size_t *product);


//
// C99 introduces %zd etc. for printing size_t, but older compilers
// especially from Microsoft may not have it.
// 

#if !defined(SIZE_T_FMT)

#if defined(_MSC_VER) && _MSC_VER < 1800
#define SIZE_T_FMT_d "u"
#define SIZE_T_FMT_x "x"
#else
#define SIZE_T_FMT_d "zu"
#define SIZE_T_FMT_x "zx"
#endif

#endif

//
// There seems to be some disagreement about where SIZE_MAX is
// declared and what it's called.
//

#if !defined(SIZE_MAX)
#if defined(SIZE_T_MAX)
#define SIZE_MAX SIZE_T_MAX
#else
#define SIZE_MAX (~(size_t)0)
#endif
#endif

#if !defined(SSIZE_MAX)
#if defined(SSIZE_T_MAX)
#define SSIZE_MAX SSIZE_T_MAX
#else
#define SSIZE_MAX (SIZE_MAX/2)
#endif
#endif

#if defined(__cplusplus)
}
#endif
#endif
