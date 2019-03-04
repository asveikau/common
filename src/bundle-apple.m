/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#import <Foundation/Foundation.h>

const char *
get_bundle_path_platform_specific()
{
   NSBundle *bundle = [NSBundle mainBundle];
   const char *r = NULL;
   if ([bundle bundleIdentifier])
   {
      r = [[bundle bundlePath] UTF8String];
   }
   return r;
}