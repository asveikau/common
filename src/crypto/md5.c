/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/crypto/md5.h>
#include <common/crypto/misc.h>

#include <string.h>

void md5_init(struct md5_state *state)
{
   static const uint32_t initial[] =
   {
      0x67452301,
      0xefcdab89,
      0x98badcfe,
      0x10325476
   };

   crypto_hash_init(&state->base);
   memcpy(state->h, initial, sizeof(initial));
}

static void
md5_process_block(struct md5_state *state)
{
   static const uint32_t k[] =
   {
      0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
      0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
      0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
      0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
      0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
      0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
      0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
      0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
      0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
      0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
      0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
      0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
      0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
      0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
      0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
      0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
   };
   static const uint32_t shift[] =
   {
      7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
      5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
      4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
      6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
   };

   uint32_t i;
   uint32_t r[ARRAY_SIZE(state->h)];
   uint32_t *buf = state->base.buffer;

   memcpy(r, state->h, sizeof(r));

   for (i=0; i<16; ++i)
      buf[i] = le32_swap(buf[i]);

   for (i=0; i<64; ++i)
   {
      uint32_t t, f, g;

      if (i < 16)
      {
         f = (r[1] & r[2]) | ((~r[1]) & r[3]);
         g = i;
      }
      else if (i < 32)
      {
         f = (r[3] & r[1]) | ((~r[3]) & r[2]);
         g = (5 * i + 1) % 16;
      }
      else if (i < 48)
      {
         f = r[1] ^ r[2] ^ r[3];
         g = (3 * i + 5) % 16;
      }
      else
      {
         f = r[2] ^ (r[1] | (~r[3]));
         g = (7 * i) % 16;
      }

      t = r[3];
      r[3] = r[2];
      r[2] = r[1];
      r[1] += rotate_left(r[0] + f + k[i] + buf[g], shift[i]);
      r[0] = t;
   }

   for (i=0; i<ARRAY_SIZE(r); ++i)
      state->h[i] += r[i];
}

DECLARE_HASH_IMPL(md5)

void
md5_final(
   struct md5_state *state,
   void **digest,
   size_t *len
)
{
   uint64_t length = le64_swap(state->base.length * 8);
   const void *pad = NULL;
   size_t padlen = 0;
   int i;

   crypto_hash_get_padding(&state->base, &pad, &padlen);
   md5_hash_data(state, pad, padlen);
   md5_hash_data(state, &length, sizeof(length));

   for (i=0; i<ARRAY_SIZE(state->h); ++i)
      state->h[i] = le32_swap(state->h[i]);

   *digest = state->h;
   *len = sizeof(state->h);
}
