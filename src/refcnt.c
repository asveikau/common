/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/refcnt.h>
#include <common/cas.h>

void
refcnt_inc(refcnt *p)
{
   refcnt old;
   do
   {
      old = *p;
   } while (!compare_and_swap(p, old, old + 1));
}

int
refcnt_dec(refcnt *p)
{
   refcnt old;
   do
   {
      old = *p;
   } while (!compare_and_swap(p, old, old - 1));
   return (old == 1);
}

void
refcnt_inc_unsafe(refcnt *p)
{
   ++*p;
}

int
refcnt_dec_unsafe(refcnt *p)
{
   return !--*p;
}
