/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_time_h_
#define common_time_h_

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

uint64_t
get_current_time();

uint64_t
get_monotonic_time_millis();

#if defined(__cplusplus)
}
#endif
#endif
