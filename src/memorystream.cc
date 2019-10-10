/*
 Copyright (C) 2019 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/c++/stream.h>
#include <common/c++/new.h>
#include <common/c++/lock.h>
#include <common/cas.h>
#include <common/misc.h>

#include <vector>

static void
AccessDenied(error *err)
{
#if defined(_WINDOWS)
   ERROR_SET(err, win32, ERROR_ACCESS_DENIED);
#else
   ERROR_SET(err, errno, EACCES);
#endif
exit:;
}

void *
common::MemoryStreamBuffer::GetBuffer()
{
   return nullptr;
}

size_t
common::MemoryStreamBuffer::GetSize()
{
   return 0;
}

void
common::MemoryStreamBuffer::Resize(size_t sz, error *err)
{
   AccessDenied(err);
}

common::MemoryStreamBuffer::MemoryStreamBuffer() : Writeable(false) {}
common::MemoryStreamBuffer::~MemoryStreamBuffer() {}

namespace
{

struct MemoryStreamBufferConst : public common::MemoryStreamBuffer
{
   void *ptr;
   size_t len;
   std::function<void(void *)> freeFn;

   void *GetBuffer() { return ptr; }
   size_t GetSize()  { return len; }

   ~MemoryStreamBufferConst()
   {
      if (freeFn && ptr) freeFn(ptr);
   }
};

struct MemoryStreamBufferVector : public common::MemoryStreamBuffer
{
   std::vector<char> vec;

   MemoryStreamBufferVector() { Writeable = true; }

   void *GetBuffer() { return vec.data(); }
   size_t GetSize()  { return vec.size(); }

   void
   Resize(size_t len, error *err)
   {
      try
      {
         vec.resize(len);
      }
      catch (std::bad_alloc)
      {
         ERROR_SET(err, nomem);
      }
   exit:;
   }
};

struct MemoryStream : public common::PStream
{
   common::MemoryStreamBuffer *mem;
   size_t len;
   common::MemoryStreamLockSheme locking;
   mutex resizeLock;    // TODO: switch to rwlock
   mutex writeLock;

   MemoryStream() : mem(nullptr), len(0), locking(common::NoSynchronization) {}
   ~MemoryStream()
   {
      if (mem)
         delete mem;

      switch (locking)
      {
      case common::NoSynchronization:
         break;
      case common::MultipleWriter:
         mutex_destroy(&writeLock);
         // fall through
      case common::SingleWriter:
         mutex_destroy(&resizeLock);
      }
   }

   void
   SynchronizeForRead(common::locker &l)
   {
      switch (locking)
      {
      case common::NoSynchronization:
         break;
      case common::SingleWriter:
      case common::MultipleWriter:
         l.acquire(resizeLock);
      }
   }

   void
   SynchronizeForResize(common::locker &l)
   {
      SynchronizeForRead(l);
   }

   void
   SynchronizeForWrite(common::locker &l)
   {
      switch (locking)
      {
      case common::MultipleWriter:
         l.acquire(writeLock);
         break;
      case common::NoSynchronization:
      case common::SingleWriter:
         break;
      }
   }

   void
   Initialize(
      common::MemoryStreamBuffer *&buf,
      common::MemoryStreamLockSheme locking,
      error *err
   )
   {
      mutex *mutices[2];
      int mi = 0;

      len = buf ? buf->GetSize() : 0;
      mem = buf;
      buf = nullptr;

      switch (locking)
      {
      case common::NoSynchronization:
         break;
      case common::MultipleWriter:
         mutex_init(&writeLock, err);
         ERROR_CHECK(err);
         mutices[mi++] = &writeLock;
         // fall through
      case common::SingleWriter:
         mutex_init(&resizeLock, err);
         ERROR_CHECK(err);
         mutices[mi++] = &resizeLock;
      }

      this->locking = locking;
      mi = 0;

      if (!mem)
      {
         CreateMemoryStreamBuffer(0, mem, err);
         ERROR_CHECK(err);
      }
   exit:
      if (ERROR_FAILED(err))
      {
         for (int i=mi-1; i >= 0; --i)
            mutex_destroy(mutices[i]);
      }
   }

   uint64_t
   GetSize(error *err)
   {
      return len;
   }

   void
   Truncate(uint64_t length, error *err)
   {
      common::locker writeLock;

      if (!mem->Writeable)
      {
         AccessDenied(err);
         goto exit;
      }

      SynchronizeForWrite(writeLock);

      if (length > mem->GetSize())
      {
         common::locker resizeLock;

         SynchronizeForResize(resizeLock);

         mem->Resize(length, err);
         ERROR_CHECK(err);

         goto clearDelta;
      }
      else if (length > this->len)
      {
      clearDelta:
         memset((char*)mem->GetBuffer() + this->len, 0, length - this->len);
         memory_barrier();
      }

      this->len = length;
   exit:;
   }

   int
   Read(void *buf, int len, uint64_t pos, error *err)
   {
      common::locker readLock;

      if (!len)
         return 0;
      if (len < 0)
      {
         error_set_unknown(err, "Negative length");
         return 0;
      }

      SynchronizeForRead(readLock);

      auto size = mem->GetSize();
      if (pos >= size)
         return 0;

      int r = (MIN(pos + len, size)) - pos;
      memcpy(buf, (char*)mem->GetBuffer() + pos, r);
      return r;
   }

   int
   Write(const void *buf, int len, uint64_t pos, error *err)
   {
      common::locker writeLock;

      if (!mem->Writeable)
      {
         AccessDenied(err);
         goto exit;
      }

      if (!len)
         goto exit;
      if (len < 0)
         ERROR_SET(err, unknown, "Negative length");

      SynchronizeForWrite(writeLock);

      if (pos+len > mem->GetSize())
      {
         common::locker resizeLock;

         SynchronizeForResize(resizeLock);
         mem->Resize(pos + len, err);
         ERROR_CHECK(err);
      }

      memcpy((char*)mem->GetBuffer() + pos, buf, len);
      memory_barrier();
      if (pos+len > len)
         this->len = pos+len;

      return len;
   exit:
      return 0;
   }
};

} // end namespace

void
common::CreateMemoryStreamBufferConst(
   const void *buf,
   size_t len,
   const std::function<void(void *)> &freeFn,
   common::MemoryStreamBuffer *&obj,
   error *err
)
{
   MemoryStreamBufferConst *p = nullptr;

   New(&p, err);
   ERROR_CHECK(err);

   p->ptr = (void*)buf;
   p->len = len;
   try
   {
      p->freeFn = freeFn;
   }
   catch (std::bad_alloc)
   {
      ERROR_SET(err, nomem);
   }

exit:
   if (ERROR_FAILED(err) && p)
   {
      delete p;
      p = nullptr;
   }
   obj = p;
}

void
common::CreateMemoryStreamBuffer(
   size_t initialSize,
   common::MemoryStreamBuffer *&obj,
   error *err
)
{
   MemoryStreamBufferVector *p = nullptr;

   New(&p, err);
   ERROR_CHECK(err);

   if (initialSize)
   {
      p->Resize(initialSize, err);
      ERROR_CHECK(err);
   }

exit:
   if (ERROR_FAILED(err) && p)
   {
      delete p;
      p = nullptr;
   }
   obj = p;
}

void
common::CreateMemoryStream(
   common::MemoryStreamBuffer *&buf,
   MemoryStreamLockSheme locking,
   PStream **out,
   error *err
)
{
   common::Pointer<MemoryStream> p;

   New(p.GetAddressOf(), err);
   ERROR_CHECK(err);

   p->Initialize(buf, locking, err);
   ERROR_CHECK(err);

exit:
   if (ERROR_FAILED(err))
      p = nullptr;
   *out = p.Detach();
}
