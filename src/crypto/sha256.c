/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/crypto/sha256.h>
#include <common/crypto/misc.h>
#include <string.h>

#define shift_right(A,B) ((A) >> (B))

void
sha256_init(struct sha256_state *state)
{
   static const uint32_t initial[] =
   {
      0x6a09e667,
      0xbb67ae85,
      0x3c6ef372,
      0xa54ff53a,
      0x510e527f,
      0x9b05688c,
      0x1f83d9ab,
      0x5be0cd19
   };
   crypto_hash_init(&state->base);
   memcpy(&state->h, initial, sizeof(initial));
}

static void
sha256_process_block(struct sha256_state *state)
{
   int i;
   uint32_t *buf = state->buffer;
   uint32_t h[ARRAY_SIZE(state->h)];

   static const uint32_t k[] =
   {
      0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
      0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
      0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
      0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
      0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
      0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
      0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
      0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
      0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
      0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
      0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
   };

   for (i=0; i<16; ++i)
      buf[i] = be32_swap(state->base.buffer[i]);

   for (i=16; i<ARRAY_SIZE(state->buffer); ++i)
   {
      uint32_t s0 =
         rotate_right(buf[i - 15], 7) ^
         rotate_right(buf[i - 15], 18) ^
         shift_right(buf[i - 15], 3);
      uint32_t s1 = 
         rotate_right(buf[i - 2], 17) ^
         rotate_right(buf[i - 2], 19) ^
         shift_right(buf[i - 2], 10);
      buf[i] = buf[i - 16] + s0 + buf[i - 7] + s1;
   }

   memcpy(h, state->h, sizeof(h));

   for (i=0; i<ARRAY_SIZE(state->buffer); ++i)
   {
      uint32_t s1 = rotate_right(h[4], 6) ^
                    rotate_right(h[4], 11) ^
                    rotate_right(h[4], 25);
      uint32_t ch = (h[4] & h[5]) ^ ((~h[4]) & h[6]);
      uint32_t temp1 =
                    h[7] + s1 + ch + k[i] + buf[i];
      uint32_t s0 = rotate_right(h[0], 2) ^
                    rotate_right(h[0], 13) ^
                    rotate_right(h[0], 22);
      uint32_t maj = (h[0] & h[1]) ^
                     (h[0] & h[2]) ^
                     (h[1] & h[2]);
      uint32_t temp2 = s0 + maj;

      h[7] = h[6];
      h[6] = h[5];
      h[5] = h[4];
      h[4] = h[3] + temp1;
      h[3] = h[2];
      h[2] = h[1];
      h[1] = h[0];
      h[0] = temp1 + temp2;
   }

   for (i=0; i < ARRAY_SIZE(h); ++i)
   {
      state->h[i] += h[i];
   }
}

DECLARE_HASH_IMPL(sha256)

void
sha256_final(
   struct sha256_state *state,
   void **digest,
   size_t *len
)
{
   uint64_t length = be64_swap(state->base.length * 8);
   const void *pad = NULL;
   size_t padlen = 0;
   int i;

   crypto_hash_get_padding(&state->base, &pad, &padlen);
   sha256_hash_data(state, pad, padlen);
   sha256_hash_data(state, &length, sizeof(length));

   for (i=0; i<ARRAY_SIZE(state->h); ++i)
      state->h[i] = be32_swap(state->h[i]);

   *digest = state->h;
   *len = sizeof(state->h);
}
