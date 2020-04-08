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
#include <functional>

namespace common {

struct StreamInfo
{
   bool IsRemote;
   bool FileSizeKnown;

   StreamInfo() : IsRemote(false), FileSizeKnown(true)
   {
   }
};

struct StreamBase : public RefCountable
{
   virtual uint64_t GetSize(error *err) = 0;
   virtual void Flush(error *err) {}
   virtual void Truncate(uint64_t length, error *err);
   virtual void GetStreamInfo(StreamInfo *, error *err) {}
};

struct PStream;

struct Stream : public StreamBase
{
   virtual uint64_t GetPosition(error *err) = 0;
   virtual void Seek(int64_t pos, int whence, error *err) = 0;
   virtual size_t Read(void *buf, size_t len, error *err) = 0;
   virtual size_t Write(const void *buf, size_t len, error *err);
   virtual void ToPStream(PStream **out, error *err);
   virtual void Substream(uint64_t pos, uint64_t len, Stream **out, error *err);
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
   virtual size_t Read(void *buf, size_t len, uint64_t pos, error *err) = 0;
   virtual size_t Write(const void *buf, size_t len, uint64_t pos, error *err);

   virtual void ToStream(Stream** out, error *err);
   virtual void Substream(uint64_t pos, uint64_t len, PStream **out, error *err);
};

void
CreateStream(
   const char *filename,
   OpenMode mode,
   PStream **out,
   error *err
);

struct MemoryStreamBuffer
{
   virtual void *GetBuffer();
   virtual size_t GetSize();
   virtual void Resize(size_t sz, error *err);
   virtual ~MemoryStreamBuffer();

   bool Writeable;

   MemoryStreamBuffer();
   MemoryStreamBuffer(const MemoryStreamBuffer&) = delete;
};

void
CreateMemoryStreamBufferConst(
   const void *buf,
   size_t len,
   const std::function<void(void *)> &freeFn,
   MemoryStreamBuffer *&obj,
   error *err
);

inline void
CreateMemoryStreamBufferConst(
   const void *buf,
   size_t len,
   MemoryStreamBuffer *&obj,
   error *err
)
{
   CreateMemoryStreamBufferConst(buf, len, std::function<void(void*)>(), obj, err);
}

void
CreateMemoryStreamBuffer(
   size_t initialSize,
   MemoryStreamBuffer *&obj,
   error *err
);

enum MemoryStreamLockSheme
{
   NoSynchronization,
   SingleWriter,
   MultipleWriter,
};

void
CreateMemoryStream(
   MemoryStreamBuffer *&buf,
   MemoryStreamLockSheme locking,
   PStream **out,
   error *err
);

inline void
CreateMemoryStream(MemoryStreamLockSheme locking, PStream **out, error *err)
{
   MemoryStreamBuffer *buf = nullptr;
   CreateMemoryStream(buf, locking, out, err);
}

} // namespace

#endif
