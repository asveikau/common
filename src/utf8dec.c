/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/utf.h>

extern unsigned char
utf8_make_high_1s(int n);

// Count the most-signifcant 1 bits that we see before a 0.
// This helps us determine the length of a multi-byte UTF-8 char based
// on the first char in the sequence.
//
static int
count_high_1s(unsigned char c)
{
   static const unsigned char table[] =
   {
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
      1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
      2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
      4,4,4,4,4,4,4,4,5,5,5,5,6,6,7,8
   };
   const int first = 128;

   if (c < first)
      return 0;
   else
      return table[c - first];
}

uint32_t
utf8_decode(const char **s)
{
   const unsigned char *p = (const unsigned char *)*s;
   uint32_t r = 0;

   switch (*p)
   {
   case 0:
      return 0;
   case 0xc0:
   case 0xc1:
   case 0xf5:
      return -1;
   }

   if (*p < 0x80)
   {
      r = *p++;
   }
   else if (*p < 0xc2)
   {
      return -1;
   }
   else
   {
      int bytes_needed = count_high_1s(*p);

      if (bytes_needed > 4)
         return -1;

      r = (~utf8_make_high_1s(bytes_needed)) & *p++;
      while (--bytes_needed)
      {
         if ((*p & 0xc0) != 0x80)
            return -1;
         r <<= 6;
         r |= ((*p++) & 0x3f);
      }
   }

   *s = (const char*)p;
   return r;
}
