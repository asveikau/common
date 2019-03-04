/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef cas_h
#define cas_h

#if defined(_WINDOWS)
#include <windows.h>
static
#if defined(__cplusplus)
inline
#else
__declspec(inline)
#endif
int
compare_and_swap(
   volatile unsigned long *ptr,
   unsigned long comparand,
   unsigned long value
   )
{
   return InterlockedCompareExchange(ptr, value, comparand) == comparand;
}
#define memory_barrier MemoryBarrier
#elif \
   (defined(__GNUC__) && \
      !(__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 2)))
#define compare_and_swap __sync_bool_compare_and_swap
#define memory_barrier __sync_synchronize
#elif defined(__arm__) && defined(__GNUC__)
static __inline__ int
compare_and_swap(
   volatile unsigned long *ptr,
   unsigned long comparand,
   unsigned long value
   )
{
   unsigned long r;
   asm __volatile__(
     "ldrex %4, [%1]\n"
     "cmp %4, %2\n"
     "strexeq %0, %3, [%1]\n"
     "dmb\n"
     : "=&r" (r), "+r" (ptr)
     : "r" (comparand), "r" (value), "r" (0), "0" (0)
     : "memory", "cc");
   return r;
}
#define memory_barrier() asm __volatile__("dmb\n" ::: "memory")
#elif defined(__ppc__) && defined(__GNUC__)
static __inline__ int
compare_and_swap(
   volatile unsigned long *ptr,
   unsigned long comparand,
   unsigned long value
   )
{
   unsigned long r;
   asm __volatile__(
     "lwarx %4, 0, %1\n"
     "cmpw %4, %2\n"
     "bne .skip%=\n"
     "stwcx. %3, 0, %1\n"
     "bne .skip%=\n"
     "li %0, 1\n"
     ".skip%=:\n"
     : "=&r" (r), "+r" (ptr)
     : "r" (comparand), "r" (value), "r" (0), "0" (0)
     : "memory", "cc");
   return r;
}
#define memory_barrier() asm __volatile__("lwsync\n" ::: "memory")
#else
#error Not ported yet.
#endif

#endif
