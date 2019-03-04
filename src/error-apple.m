/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/error.h>
#include <string.h>
#import <Foundation/Foundation.h>

const char *
oserror_get_string(error *err)
{
   NSError *error =
      [NSError errorWithDomain:NSOSStatusErrorDomain
               code:err->code
               userInfo:nil];
   NSString *str = [error description];
   const char *r = [str UTF8String];
   if (r)
      r = strchr(r, '"');
   if (r && !strcmp(r, "\"(null)\""))
      r = NULL;
   return r;
}

void
error_set_osstatus(error *err, OSStatus status)
{
   error_clear(err);
   err->source = ERROR_SRC_OSSTATUS;
   err->code = status;
   err->get_string = oserror_get_string;
}

void
error_set_darwin(error *err, int ret)
{
   error_clear(err);
   err->source = ERROR_SRC_DARWIN;
   err->code = ret;
   // TODO: no stringify for this at this time
}