/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/buffer.h>
#include <common/size.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void *
buffer_alloc(buffer *buf, size_t len)
{
   size_t alloc = buf->alloc;
   char *p = NULL;

   if (!alloc)
      alloc = 32;

   while ((alloc - buf->len) < len)
   {
      if (size_mult(alloc, 2, &alloc))
         return NULL;
   }

   if (alloc != buf->alloc)
   {
      void *ptr;
      if (buf->buf)
         ptr = realloc(buf->buf, alloc);
      else
         ptr = malloc(alloc);
      if (!ptr)
         return NULL;
      buf->buf = ptr;
      buf->alloc = alloc;
   }

   p = buf->buf;
   p += buf->len;
   buf->len += len;
   return p;
}

void *
buffer_append(buffer *buf, const void *src, size_t len)
{
   void *p = buffer_alloc(buf, len);
   if (p)
      memcpy(p, src, len);
   return p;
}

void
buffer_remove(buffer *buf, size_t start, size_t len)
{
   size_t endoff = 0;

   if (start > buf->len)
      return;

   assert(!size_add(start, len, &endoff));
   if (size_add(start, len, &endoff))
   {
      endoff = buf->len;
   }

   if (endoff >= buf->len)
   {
      // Remove at tail.  Just update length.
      //
      buf->len = start;
   }
   else
   {
      char *p = buf->buf;
      memmove(p + start, p + endoff, buf->len - endoff);
      buf->len -= (endoff - start);
   }
}

void
buffer_destroy(buffer *buf)
{
   if (buf)
   {
      free(buf->buf);
      memset(buf, 0, sizeof(*buf));
   }
}

