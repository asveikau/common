/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/crypto/sha1.h>
#include <common/crypto/misc.h>
#include <string.h>

void
sha1_init(struct sha1_state *state)
{
   static const uint32_t initial[] =
   {
      0x67452301,
      0xEFCDAB89,
      0x98BADCFE,
      0x10325476,
      0xC3D2E1F0
   };
   crypto_hash_init(&state->base);
   memcpy(&state->h, initial, sizeof(initial));
}

static void
sha1_process_block(struct sha1_state *state)
{
   int i;
   uint32_t *buf = state->buffer;
   uint32_t h[ARRAY_SIZE(state->h)];
   uint32_t f, k;

   for (i=0; i<16; ++i)
      buf[i] = be32_swap(state->base.buffer[i]);

   for (i=16; i<ARRAY_SIZE(state->buffer); ++i)
      buf[i] = rotate_left(buf[i-3] ^ buf[i-8] ^ buf[i-14] ^ buf[i-16], 1);

   memcpy(h, state->h, sizeof(h));

   for (i=0; i<ARRAY_SIZE(state->buffer); ++i)
   {
      if (i < 20)
      {
         f = (h[1] & h[2]) | ((~h[1]) & h[3]);
         k = 0x5a827999;
      }
      else if (i < 40)
      {
         f = (h[1] ^ h[2] ^ h[3]);
         k = 0x6ed9eba1;
      }
      else if (i < 60)
      {
         f = (h[1] & h[2]) | (h[1] & h[3]) | (h[2] & h[3]);
         k = 0x8f1bbcdc;
      }
      else
      {
         f = h[1] ^ h[2] ^ h[3];
         k = 0xca62c1d6;
      }
      k += rotate_left(h[0], 5) + f + h[4] + buf[i];
      h[4] = h[3];
      h[3] = h[2];
      h[2] = rotate_left(h[1], 30);
      h[1] = h[0];
      h[0] = k;
   }

   for (i=0; i < ARRAY_SIZE(h); ++i)
   {
      state->h[i] += h[i];
   }
}

DECLARE_HASH_IMPL(sha1)

void
sha1_final(
   struct sha1_state *state,
   void **digest,
   size_t *len
)
{
   uint64_t length = be64_swap(state->base.length * 8);
   const void *pad = NULL;
   size_t padlen = 0;
   int i;

   crypto_hash_get_padding(&state->base, &pad, &padlen);
   sha1_hash_data(state, pad, padlen);
   sha1_hash_data(state, &length, sizeof(length));

   for (i=0; i<ARRAY_SIZE(state->h); ++i)
      state->h[i] = be32_swap(state->h[i]);

   *digest = state->h;
   *len = sizeof(state->h);
}
