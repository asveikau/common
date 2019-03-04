/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_backtrace_h_
#define common_backtrace_h_

#include <stddef.h>

#if defined(__linux__) || defined(__APPLE__) || \
    defined(__FreeBSD__) || defined(__OpenBSD__)
#include <execinfo.h>
#else
#define NEED_BACKTRACE
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef NEED_BACKTRACE
size_t
backtrace(void **addrs, size_t naddrs);
#endif

void
describe_symbol(void *addr, char *buf, int bufsz);

void
register_backtrace_logger(
   void (*on_crash)(void*),  // can be null
   void *ctx
);

#if defined(__cplusplus)
}
#endif
#endif
