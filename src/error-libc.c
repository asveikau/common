/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/error.h>

#if defined(_MSC_VER)
#define USE_STRERROR_S
#endif

#if defined(USE_STRERROR_S)
#define __STDC_WANT_LIB_EXT1__ 1
#endif

#if defined(__sun__)
#define USE_STRERROR_R
#endif

#include <string.h>
#include <stdlib.h>
#include <errno.h>

static const char*
errno_get_string(error *err)
{
   const char *r = NULL;

#if defined(USE_STRERROR_R) || defined(USE_STRERROR_S)
   if (err->code == ENOMEM)
      return "Out of memory";
   else if (err->context)
      r = err->context;
   else
   {
      void *p = NULL;
      int sz = 4096;

      static const char prefix[] = "Unknown error";

#if defined(USE_STRERRORLEN_S)
      sz = strerrorlen_s(err->code) + 1;
#endif

      p = malloc(sz);
      if (!p)
         return "<Out of memory formatting message>";
      memset(p, 0, sz);

#if defined(USE_STRERROR_R)
      strerror_r(err->code, p, sz);
#elif defined(USE_STRERROR_S)
      strerror_s(p, sz, err->code);
#endif

      if (!strncmp(p, prefix, sizeof(prefix)-1))
      {
         free(p);
         p = NULL;
      }

      if (p)
      {
         err->context = p;
         err->free_fn = free;
      }

      r = p;
   }
#else
#if !defined(__OpenBSD__) && !defined(__NetBSD__)
   extern const char * const sys_errlist[];
   extern const int sys_nerr;
#endif

   if (err->code > 0 && err->code < sys_nerr)
      r = sys_errlist[err->code];
#endif

   return r;
}

void
error_set_errno(error *err, int code)
{
   error_clear(err);

   if (code)
   {
      err->source = ERROR_SRC_ERRNO;
      err->code = code;
      err->get_string = errno_get_string;
   }
   else
   {
      error_set_unknown(err, "Failure case, but no errno?");
   }
}
