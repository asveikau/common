/*
 Copyright (C) Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/


// Generate a byte where the N most significant bits are set.
// This is the inverse of count_high_1s.
//
unsigned char
utf8_make_high_1s(int n)
{
   static const unsigned char table[] =
   {
      // Note in UTF-8 we never use 0x00 and only make it up to 0xf0, but adding
      // all 8 for completeness...
      //
      0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff
   };

   return table[n];
}

