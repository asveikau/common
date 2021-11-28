/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef misc_h
#define misc_h

#include "error.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WINDOWS)
#include <io.h>
#include <fcntl.h>

#define stdio_set_binary(fp) _setmode(_fileno(fp), _O_BINARY)
#else
#define stdio_set_binary(fp) ((void)0)
#endif

#if defined(_WINDOWS)
#define strtoull _strtoui64
#define strtoll  _strtoi64
#endif

#ifndef MIN
#define MIN(X, Y)    (((X) < (Y)) ? (X) : (Y))
#endif

#ifndef MAX 
#define MAX(X, Y)    (((X) > (Y)) ? (X) : (Y))
#endif

#if !defined(ARRAY_SIZE)
#if defined(__cplusplus)
namespace common { namespace internal
{
   template<class T, size_t N>
   char (&SizeHelper(T (&arr)[N]))[N];
} } // end namespace
#define ARRAY_SIZE(X) sizeof(common::internal::SizeHelper(X))
#else
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(*X))
#endif
#endif

#if defined(__cplusplus)
#define INLINE inline
#elif defined(_MSC_VER)
#define INLINE __declspec(inline)
#elif defined(__GNUC__)
#define INLINE __inline__
#else
#error
#endif

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, memb) ((intptr_t)(&((type*)0)->memb))
#endif

#define CHECK_ATOI_BODY(SUFFIX, FUNC, TYPE) \
static INLINE                               \
bool                                        \
check_##SUFFIX(const char *p, TYPE *o)      \
{                                           \
   char *q = NULL;                          \
   TYPE r = FUNC(p, &q, 10);                \
   if (p != q)                              \
   {                                        \
      *o = r;                               \
      return true;                          \
   }                                        \
   *o = -1;                                 \
   return false;                            \
}

CHECK_ATOI_BODY(atoi, strtol, int)
CHECK_ATOI_BODY(atoi64, strtoll, int64_t)
CHECK_ATOI_BODY(atou64, strtoull, uint64_t)

#undef CHECK_ATOI_BODY

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__linux__) || defined(__APPLE__)
int
closefrom(int minfd);
#endif

#if defined(_WINDOWS)

PWSTR
ConvertToPwstr(PCSTR utf8, error *err);

PSTR
ConvertToPstr(PCWSTR utf16, error *err);

#define ftello _ftelli64
#define fseeko _fseeki64

int
asprintf(char **dst, const char *fmt, ...);

int
vasprintf(char **dst, const char *fmt, va_list ap);

#endif

#if !defined(PID_T_FMT)
#define PID_T_FMT "d"
#endif

#if defined(__cplusplus)
}
#endif
#endif
