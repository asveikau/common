/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef crypto_hash_h
#define crypto_hash_h

#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

struct crypto_hash
{
   uint64_t length;
   uint32_t buffer[16];
   void *current_pos;
};

void
crypto_hash_init(struct crypto_hash *obj);

int
crypto_hash_data(
   struct crypto_hash *obj,
   const void *data,
   size_t len,
   size_t *bytes_consumed
);

void
crypto_hash_get_padding(
   struct crypto_hash *obj,
   const void **buffer,
   size_t *length
);

#if defined(__cplusplus)
}

namespace common
{
template<typename T,
         void (*init)(T*),
         void (*data_)(T*, const void *buf, size_t len),
         void (*final_)(T*, void **, size_t *len)>
class hash_base
{
   T state;
public:
   hash_base() { reset(); }
   hash_base(const hash_base&t) = delete;

   void reset()
   {
      init(&state);
   }

   void data(const void *buf, size_t len)
   {
      data_(&state, buf, len);
   }

   void final(void **buffer, size_t *len)
   {
      final_(&state, buffer, len);
   }
};
} // end namespace

#endif
#endif
