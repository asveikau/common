/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef libcommon_checkatoi_h
#define libcommon_checkatoi_h

#include <stdbool.h>
#include <stdint.h>

#include "inline.h"

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

#endif
