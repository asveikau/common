/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_ring_h_
#define common_ring_h_

#include "../cas.h"
#include "../misc.h"

namespace common
{

template<typename T, int bits = 8>
class RingBuffer
{
   T buffer[1 << bits];
   volatile int reader, writer;

   int Mod(int offset)
   {
      return offset & (ARRAY_SIZE(buffer) - 1);
   }
public:

   RingBuffer() : reader(0), writer(0) {}

   bool IsEmpty() const { return reader == writer; }

   int
   Write(T *input, int n)
   {
      int o = writer, i = reader;
      int r = 0;
      while (n && Mod(o+1) != i)
      {
         buffer[o] = std::move(*input++);
         --n;
         ++r;
         o = Mod(o+1);
      }
      if (r)
      {
         memory_barrier();
         writer = o;
      }
      return r;
   }

   int
   Read(T *output, int n)
   {
      int r = 0;
      int o = writer, i = reader;
      if (i != o)
      {
         memory_barrier();
         while (n && i != o)
         {
            *output++ = std::move(buffer[i]);
            i = Mod(i+1);
            --n;
            ++r;
         }
      }
      if (r)
         reader = i;
      return r;
   }
};

} // end namespace

#endif
