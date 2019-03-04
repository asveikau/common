/*
 Copyright (C) Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef utf_h
#define utf_h

#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

uint32_t
utf16_decode(const uint16_t **s);

int
utf16_encode(uint32_t input, void *buffer, size_t bufsize);

uint32_t
utf8_decode(const char **s);

int
utf8_encode(uint32_t input, void *buffer, size_t bufsize);

#if defined(__cplusplus)
}
#endif
#endif
