/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/misc.h>

int
asprintf(char **dst, const char *fmt, ...)
{
   va_list ap;
   int r;
   va_start(ap, fmt);
   r = vasprintf(dst, fmt, ap);
   va_end(ap);
   return r;
}

int
vasprintf(char **dst, const char *fmt, va_list ap)
{
   int r = 0;   
   va_list copy;

   va_copy(copy, ap);

   r = _vscprintf(fmt, copy) + 1;

   va_end(copy);

   *dst = malloc(r);
   if (!*dst)
      return -1;
   
   return vsnprintf(*dst, r, fmt, ap);
}
