/*
 Copyright (C) 2026 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/error.h>
#include <common/stdiomisc.h>

#include <stdlib.h>

void
error_set_vfmt(error *err, const char *fmt, va_list ap)
{
   char *buf = NULL;

   vasprintf(&buf, fmt, ap);

   error_set_unknown(err, buf ? buf : "<Failed to allocate>");
   if (buf)
      err->free_fn = free;
}

void
error_set_fmt(error *err, const char *fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);
   error_set_vfmt(err, fmt, ap);
   va_end(ap);
}
