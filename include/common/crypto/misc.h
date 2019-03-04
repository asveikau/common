/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef crypto_misc_h
#define crypto_misc_h

#include <stdint.h>
#include "../misc.h"

#if defined(__cplusplus)
extern "C" {
#endif

static INLINE uint32_t
rotate_left(uint32_t src, int rot)
{
   if (rot)
      src = (src << rot) | (src >> (32 - rot));
   return src;
}

static INLINE uint32_t
rotate_right(uint32_t src, int rot)
{
   if (rot)
      src = (src >> rot) | (src << (32 - rot));
   return src;
}

static INLINE uint32_t
be32_swap(uint32_t src)
{
   uint32_t dsti;
   unsigned char *dst = (unsigned char *)&dsti;
   *dst++ = (src >> 24);
   *dst++ = (src >> 16);
   *dst++ = (src >> 8);
   *dst = src;
   return dsti;
}

static INLINE uint64_t
be64_swap(uint64_t src)
{
   uint64_t dsti;
   unsigned char *dst = (unsigned char*)&dsti;
   *dst++ = src >> 56;
   *dst++ = src >> 48;
   *dst++ = src >> 40;
   *dst++ = src >> 32;
   *dst++ = src >> 24;
   *dst++ = src >> 16;
   *dst++ = src >> 8;
   *dst = src;
   return dsti;
}

static INLINE uint32_t
le32_swap(uint32_t src)
{
   uint32_t dsti;
   unsigned char *dst = (unsigned char *)&dsti;
   *dst++ = src;
   *dst++ = (src >> 8);
   *dst++ = (src >> 16);
   *dst = (src >> 24);
   return dsti;
}

static INLINE uint64_t
le64_swap(uint64_t src)
{
   uint64_t dsti;
   unsigned char *dst = (unsigned char*)&dsti;
   *dst++ = src;
   *dst++ = src >> 8;
   *dst++ = src >> 16;
   *dst++ = src >> 24;
   *dst++ = src >> 32;
   *dst++ = src >> 40;
   *dst++ = src >> 48;
   *dst++ = src >> 56;
   return dsti;
}

#define DECLARE_HASH_IMPL(FUNC)                                \
void                                                           \
FUNC##_hash_data(                                              \
   struct FUNC##_state *state,                                 \
   const void *buf,                                            \
   size_t len                                                  \
)                                                              \
{                                                              \
   while (len)                                                 \
   {                                                           \
      size_t consumed = 0;                                     \
                                                               \
      if (crypto_hash_data(&state->base, buf, len, &consumed)) \
      {                                                        \
         FUNC##_process_block(state);                          \
      }                                                        \
                                                               \
      buf = (const char*)buf + consumed;                       \
      len -= consumed;                                         \
   }                                                           \
}

#if defined(__cplusplus)
}
#endif
#endif
