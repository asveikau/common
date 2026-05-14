/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef libcommon_closefrom_h
#define libcommon_closefrom_h

#include "glibccheck.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if (defined(__linux__) && !LIBCOMMON_GLIBC_CHECK(2,34)) || \
    defined(__APPLE__)
#define LIBCOMMON_NEED_CLOSEFROM 1
int
closefrom(int minfd);
#endif

#if defined(__cplusplus)
}
#endif

#endif
