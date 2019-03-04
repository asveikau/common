/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/crypto/hash.h>
#include <common/crypto/misc.h>
#include <string.h>

void
crypto_hash_init(struct crypto_hash *obj)
{
   obj->length = 0;
   obj->current_pos = obj->buffer;
}

int
crypto_hash_data(
   struct crypto_hash *obj,
   const void *data,
   size_t len,
   size_t *consumed
)
{
   int r = 0;
   char *start = obj->current_pos;
   char *end = (char*)(obj->buffer + ARRAY_SIZE(obj->buffer));
   size_t to_copy = MIN(len, end - start);

   if (to_copy)
   {
      memcpy(start, data, to_copy);
      obj->current_pos = (start += to_copy);
      obj->length += to_copy;
   }

   if (start == end)
   {
      r = 1;
      obj->current_pos = obj->buffer;
   }

   *consumed = to_copy;
   return r;
}

void
crypto_hash_get_padding(
   struct crypto_hash *obj,
   const void ** buffer,
   size_t *length
)
{
   static const char padding[64] = {0x80, 0};
   int l = ((obj->length * 8) % 512);
   *length = ((l < 448) ? (448 - l) : (512 - (l - 448))) / 8;
   *buffer = padding;
}
