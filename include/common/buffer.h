/*
 Copyright (C) Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef buffer_h_
#define buffer_h_

#include <stddef.h>

typedef struct
{
   void *buf;
   size_t len, alloc;
} buffer;

// Public accessors.  We add 0 to values so that they don't end up
// being used as lvalues.
//
#define BUFFER_PTR(pbuf)         ((void*)((char*)(pbuf)->buf + 0))
#define BUFFER_NBYTES(pbuf)      ((pbuf)->len + 0)
#define BUFFER_NMEMB(pbuf, type) (BUFFER_NBYTES(pbuf)/sizeof(type))

void *
buffer_alloc(buffer *, size_t);

void *
buffer_append(buffer *, const void *, size_t);

void
buffer_remove(buffer *, size_t, size_t);

void
buffer_destroy(buffer *);

#endif
