/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_cpp_stream_h
#define common_cpp_stream_h

#include "../error.h"
#include "refcount.h"
#include <stdio.h>
#include <stdint.h>

namespace common {

struct StreamBase : public RefCountable
{
   virtual uint64_t GetSize(error *err) = 0;
   virtual void Flush(error *err) {}
   virtual void Truncate(uint64_t length, error *err);
};

struct Stream : public StreamBase
{
   virtual uint64_t GetPosition(error *err) = 0;
   virtual void Seek(int64_t pos, int whence, error *err) = 0;
   virtual int Read(void *buf, int len, error *err) = 0;
   virtual int Write(const void *buf, int len, error *err);
};

enum OpenMode
{
   ReadOnly,
   ReadWrite,
   ReadWriteCreate,
};

void
CreateStream(
   const char *filename,
   const char *mode,
   Stream **out,
   error *err
);

void
CreateStreamReadWriteCreate(
   const char *filename,
   Stream **out,
   error *err
);

void
CreateStream(
   const char *filename,
   OpenMode mode,
   Stream **out,
   error *err
);

void
CreateStream(FILE *f, Stream **out, error *err);

// Like Stream, but following the model of pread().
//
struct PStream : public StreamBase
{
   virtual int Read(void *buf, int len, uint64_t pos, error *err) = 0;
   virtual int Write(const void *buf, int len, uint64_t pos, error *err);

   void ToStream(Stream** out, error *err);
};

void
CreateStream(
   const char *filename,
   OpenMode mode,
   PStream **out,
   error *err
);

} // namespace

#endif
