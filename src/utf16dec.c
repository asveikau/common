/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/utf.h>

uint32_t
utf16_decode(const uint16_t **p)
{
   const uint16_t *s = *p;
   uint32_t r = 0;

   if (!*s)
      return 0;
   else if (*s < 0xd800 || *s >= 0xe000)
      r = *s++;
   else
   {
      if ((*s & 0xfc00) != 0xd800)
         return -1;
      r = (*s++ & 0x3ff) << 10;
      if ((*s & 0xfc00) != 0xdc00)
         return -1;
      r |= (*s++ & 0x3ff);
      r += 0x10000;
   }

   *p = s;
   return r;
}
