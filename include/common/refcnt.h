/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef refcnt_h
#define refcnt_h

#if defined(__cplusplus)
extern "C" {
#endif

typedef volatile unsigned long refcnt;

void
refcnt_inc(refcnt *p);

int
refcnt_dec(refcnt *p);

void
refcnt_inc_unsafe(refcnt *p);

int
refcnt_dec_unsafe(refcnt *p);

#if defined(__cplusplus)
}
#endif

#endif
