/*
 Copyright (C) 2017-2020 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef size_h_
#define size_h_

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#if defined(_MSC_VER)
#include <intsafe.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#if defined (__GNUC__)

static __inline__
int
size_add(size_t a, size_t b, size_t *sum)
{
   return __builtin_add_overflow(a, b, sum) ? -1 : 0;
}

static __inline__
int
size_mult(size_t a, size_t b, size_t *product)
{
   return __builtin_mul_overflow(a, b, product) ? -1 : 0;
}

#elif defined(_MSC_VER)

static __inline
int
size_add(size_t a, size_t b, size_t *sum)
{
   return FAILED(SizeTAdd(a, b, sum)) ? -1 : 0;
}

static __inline
int
size_mult(size_t a, size_t b, size_t *product)
{
   return FAILED(SizeTMult(a, b, product)) ? -1 : 0;
}

#else

#define SIZE_ADDMULT_NO_INLINE 1

int
size_add(size_t a, size_t b, size_t *sum);

int
size_mult(size_t a, size_t b, size_t *product);

#endif

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
#define SSIZE_MAX ((ssize_t)(SIZE_MAX/2))
#endif
#endif

#if defined(__cplusplus)
}
#endif

//
// C++ only - utility functions to convert between size_t read functions
// and other types, like int or size_t
//
#if defined(__cplusplus)
#include <common/misc.h>
#if defined(_WINDOWS)
#include <windows.h>
#endif

namespace common
{

template<typename IOFunc, typename AdvanceFunc, typename InnerLen>
size_t
IoFuncToSizeT(
   IOFunc ioFunc,
   AdvanceFunc advanceFunc,
   size_t len,
   InnerLen max,
   error *err
)
{
   size_t total = 0;
   while (len)
   {
      auto n = MIN(max, len);
      InnerLen r = ioFunc(n, err);
      ERROR_CHECK(err);
      if (r <= 0)
         break;
      total += r;
      len -= r;
      advanceFunc(r);
      if (r < n)
         break;
   }
exit:
   return total;
}

template<typename IOFunc, typename AdvanceFunc>
size_t
IntIoFuncToSizeT(
   IOFunc ioFunc,
   AdvanceFunc advanceFunc,
   size_t len,
   error *err
)
{
   return IoFuncToSizeT(ioFunc, advanceFunc, len, (int)INT_MAX, err);
}

#if defined(_WINDOWS)

template<typename IOFunc, typename AdvanceFunc>
size_t
DWordIoFuncToSizeT(
   IOFunc ioFunc,
   AdvanceFunc advanceFunc,
   size_t len,
   error *err
)
{
   return IoFuncToSizeT(ioFunc, advanceFunc, len, (DWORD)~0U, err);
}

#endif

#if !defined(_MSC_VER)

template<typename IOFunc, typename AdvanceFunc>
size_t
SSizeTIoFuncToSizeT(
   IOFunc ioFunc,
   AdvanceFunc advanceFunc,
   size_t len,
   error *err
)
{
   return IoFuncToSizeT(ioFunc, advanceFunc, len, (ssize_t)SSIZE_MAX, err);
}

#endif

} // end namespace
#endif

#endif
