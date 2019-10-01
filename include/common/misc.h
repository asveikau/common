/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef misc_h
#define misc_h

#include <stdint.h>
#include "error.h"

#include <stdio.h>
#include <stdarg.h>

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

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef MIN
#define MIN(X, Y)    (((X) < (Y)) ? (X) : (Y))
#endif

#ifndef MAX 
#define MAX(X, Y)    (((X) > (Y)) ? (X) : (Y))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(*X))
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

#if !defined(SIZE_T_FMT)

#if defined(_MSC_VER) && _MSC_VER < 1800
#define SIZE_T_FMT_d "u"
#define SIZE_T_FMT_x "x"
#else
#define SIZE_T_FMT_d "zu"
#define SIZE_T_FMT_x "zx"
#endif

#endif

#if !defined(PID_T_FMT)
#define PID_T_FMT "d"
#endif

#if defined(__cplusplus)
}
#endif
#endif
