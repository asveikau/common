/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/utf.h>

#include <string.h>

extern unsigned char
utf8_make_high_1s(int n);

int
utf8_encode(uint32_t input, void *bufferp, size_t bufsize)
{
   size_t nbytes = 0;
   unsigned char *buffer = bufferp;
   unsigned char *p;

   if (input < 0x80)
   {
      if (!bufsize)
         return -1;
      *buffer = input;
      return 1;
   }
   else if (input < 0x800)
      nbytes = 2;
   else if (input < 0x10000)
      nbytes = 3;
   else if (input < 0x110000)
      nbytes = 4;
   else
      return -1;

   if (bufsize < nbytes)
      return -1;

   buffer[0] = utf8_make_high_1s(nbytes);
   memset(buffer + 1, 0, nbytes - 1);
   for (p = buffer + nbytes - 1; p >= buffer; --p)
   {
      *p |= (0x80 | (input & 0x3f));
      input >>= 6;
   }

   return nbytes;
}

