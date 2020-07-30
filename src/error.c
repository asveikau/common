/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/error.h>
#include <common/logger.h>

#include <string.h>
#include <stdio.h>

const char *
error_get_string(error *err)
{
   if (err->get_string)
      return err->get_string(err);
   return NULL;
}

static const char*
unknown_get_string(error *err)
{
   return err->context ? err->context : "<Generic error>";
}

void
error_set_unknown(error *err, const char *msg)
{
   error_clear(err);
   err->source = ERROR_SRC_UNKNOWN;
   err->get_string = unknown_get_string;
   err->context = (char*)msg;
}

void
error_set_nomem(error *err)
{
#if defined(_WINDOWS)
   error_set_win32(err, E_OUTOFMEMORY);
#else
   error_set_errno(err, ENOMEM);
#endif
}

void
error_set_notimpl(error *err)
{
#if defined(_WINDOWS)
   error_set_win32(err, E_NOTIMPL);
#else
   error_set_errno(err, ENOTSUP);
#endif
}

void
error_set_access(error *err)
{
#if defined(_WINDOWS)
   error_set_win32(err, ERROR_ACCESS_DENIED);
#else
   error_set_errno(err, EACCES);
#endif
}

void
error_clear(error *err)
{
   if (err)
   {
      if (err->free_fn)
         err->free_fn(err->context);

      memset(err, 0, sizeof(*err));
   }
}

void 
error_log(error *err, const char *func, const char *file, int line_no)
{
   const char *str = error_get_string(err);
   const char *source = NULL;
   char hexbuf[10];

   switch (err->source)
   {
   case ERROR_SRC_ERRNO:
      source = "errno";
      break;
   case ERROR_SRC_COM:
      source = "com";
      break;
   case ERROR_SRC_OSSTATUS:
      source = "osstatus";
      break;
   case ERROR_SRC_DARWIN:
      source = "kern_return_t";
      break;
   case ERROR_SRC_UNKNOWN:
      source = "generic";
      break;
   default:
      snprintf(hexbuf, sizeof(hexbuf), "%x", err->source);  
      source = hexbuf; 
   }

   log_printf(
      "%s (%s:%d): %s 0x%.8x%s%s",
      func, file, line_no,
      source,
      err->code,
      str ? ": " : "",
      str ? str : ""
   );
}

