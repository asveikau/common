/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/utf.h>

int
utf16_encode(uint32_t input, void *bufferp, size_t bufsize)
{
   uint16_t *buffer = bufferp;

   if (input >= 0xd800 && input < 0xe000)
      return -1;
   else if (input >= 0x10000 && input < 0x110000)
   {
      if (bufsize < 2)
         return -1;

      input -= 0x10000; 
      buffer[1] = 0xdc00 | (input & 0x3ff);
      input >>= 10;
      buffer[0] = 0xd800 | (input & 0x3ff);

      return 2;
   }
   else
   {
      if (bufsize < 1)
         return -1;

      *buffer = input;
      return 1;
   }
}
