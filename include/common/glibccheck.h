/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef libcommon_glibc_check
#define libcommon_glibc_check

#define LIBCOMMON_GLIBC_CHECK(MAJOR,MINOR)                 \
  (__GLIBC__ &&                                            \
     (__GLIBC__ > MAJOR ||                                 \
        (__GLIBC__ == MAJOR && __GLIBC_MINOR__ >= MINOR)))

#endif
