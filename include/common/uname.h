/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_uname_h_
#define common_uname_h_

#if defined(_WINDOWS)

#if defined(__cplusplus)
extern "C" {
#endif

#define _SYS_NAMELEN 32

struct utsname
{
   char sysname[_SYS_NAMELEN];
   char nodename[_SYS_NAMELEN];
   char release[_SYS_NAMELEN];
   char version[_SYS_NAMELEN];
   char machine[_SYS_NAMELEN];
};

int
uname(struct utsname *buf);

#if defined(__cplusplus)
}
#endif

#else
#include <sys/utsname.h>
#endif

#endif
