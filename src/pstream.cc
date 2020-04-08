/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/c++/stream.h>
#include <common/c++/new.h>
#include <common/path.h>
#include <common/misc.h>
#include <common/size.h>

#if defined(_WINDOWS)
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace {
#if defined(_WINDOWS)

struct WinPStream : public common::PStream
{
   HANDLE handle;

   WinPStream() : handle(INVALID_HANDLE_VALUE) {}
   ~WinPStream() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);}

   void
   open(const char *filename, common::OpenMode mode, error *err)
   {
      DWORD desiredAccess = GENERIC_READ;
      DWORD disposition = OPEN_EXISTING;
      DWORD flagsAndAttrs = FILE_ATTRIBUTE_NORMAL;
      PWSTR utf16 = nullptr;

      switch (mode)
      {
      case common::OpenMode::ReadOnly:
         break;
      case common::OpenMode::ReadWriteCreate:
         disposition = OPEN_ALWAYS;
         // fall through ...
      case common::OpenMode::ReadWrite:
         desiredAccess |= GENERIC_WRITE;
         break;
      default:
         ERROR_SET(err, unknown, "unrecognized mode");
      }

      utf16 = ConvertToPwstr(filename, err);
      ERROR_CHECK(err);

      handle = CreateFile(
         utf16,
         desiredAccess,
         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
         nullptr,
         disposition,
         flagsAndAttrs,
         nullptr
      );
      if (handle == INVALID_HANDLE_VALUE)
         ERROR_SET(err, win32, GetLastError());
   exit:
      free(utf16);
   }

   uint64_t
   GetSize(error *err)
   {
      LARGE_INTEGER r = {0};

      if (!GetFileSizeEx(handle, &r))
         ERROR_SET(err, win32, GetLastError());
   exit:
      return r.QuadPart;
   }

   size_t
   Read(void *buf, size_t len, uint64_t pos, error *err)
   {
      return common::DWordIoFuncToSizeT(
         [&] (DWORD n, error *err) -> DWORD
         {
            ULARGE_INTEGER uli = {0};
            OVERLAPPED ol = {0};
            DWORD out = 0;

            uli.QuadPart = pos;
            ol.Offset = uli.LowPart;
            ol.OffsetHigh = uli.HighPart;

            if (!ReadFile(handle, buf, n, &out, &ol))
               ERROR_SET(err, win32, GetLastError());
         exit:
            return out;
         },
         [&] (DWORD n) -> void
         {
            buf = (char*)buf + n;
            pos += n;
         },
         len,
         err
      );
   }

   size_t
   Write(const void *buf, size_t len, uint64_t pos, error *err)
   {
      return common::DWordIoFuncToSizeT(
         [&] (DWORD n, error *err) -> DWORD
         {
            ULARGE_INTEGER uli = {0};
            OVERLAPPED ol = {0};
            DWORD out = 0;

            uli.QuadPart = pos;
            ol.Offset = uli.LowPart;
            ol.OffsetHigh = uli.HighPart;

            if (!WriteFile(handle, buf, n, &out, &ol))
               ERROR_SET(err, win32, GetLastError());
         exit:
            return out;
         },
         [&] (DWORD n) -> void
         {
            buf = (const char*)buf + n;
            pos += n;
         },
         len,
         err
      );
   }

   void
   Truncate(uint64_t length, error *err)
   {
      LARGE_INTEGER li = {0};
      LARGE_INTEGER out = {0};

      li.QuadPart = length;
      if (!SetFilePointerEx(handle, li, &out, FILE_BEGIN))
         ERROR_SET(err, win32, GetLastError());
      if (!SetEndOfFile(handle))
         ERROR_SET(err, win32, GetLastError());
   exit:;
   }

   void
   GetStreamInfo(common::StreamInfo *info, error *err)
   {
      info->IsRemote = fd_is_remote(handle, err);
      ERROR_CHECK(err);
   exit:;
   }
};

typedef WinPStream PlatformPStream;

#else

struct UnixPStream : public common::PStream
{
   int fd;

   UnixPStream() : fd(-1) {}
   ~UnixPStream() { if (fd >= 0) close(fd); }

   void
   open(const char *filename, common::OpenMode mode, error *err)
   {
      int unix_mode;

      switch (mode)
      {
      case common::OpenMode::ReadOnly:
         unix_mode = O_RDONLY;
         break;
      case common::OpenMode::ReadWrite:
         unix_mode = O_RDWR;
         break;
      case common::OpenMode::ReadWriteCreate:
         unix_mode = O_RDWR | O_CREAT;
         break;
      default:
         ERROR_SET(err, unknown, "unrecognized mode");
      }

      fd = ::open(filename, unix_mode, 0666);
      if (fd < 0)
         ERROR_SET(err, errno, errno);
   exit:;
   }

   uint64_t
   GetSize(error *err)
   {
      uint64_t r = 0;
      struct stat buf;
      if (fstat(fd, &buf))
         ERROR_SET(err, errno, errno);
      r = buf.st_size;
   exit:
      return r;
   }

   size_t
   Read(void *buf, size_t len, uint64_t pos, error *err)
   {
      return common::SSizeTIoFuncToSizeT(
         [&] (size_t n, error *err) -> ssize_t
         {
            auto r = pread(fd, buf, n, pos);
            if (r < 0)
               ERROR_SET(err, errno, errno);
         exit:
            return r;
         },
         [&] (ssize_t n) -> void
         {
            buf = (char*)buf + n;
            pos += n;
         },
         len,
         err
      );
   }

   size_t
   Write(const void *buf, size_t len, uint64_t pos, error *err)
   {
      return common::SSizeTIoFuncToSizeT(
         [&] (size_t n, error *err) -> ssize_t
         {
            auto r = pwrite(fd, buf, n, pos);
            if (r < 0)
               ERROR_SET(err, errno, errno);
         exit:
            return r;
         },
         [&] (ssize_t n) -> void
         {
            buf = (const char*)buf + n;
            pos += n;
         },
         len,
         err
      );
   }

   void
   Truncate(uint64_t length, error *err)
   {
      if (ftruncate(fd, length))
         ERROR_SET(err, errno, errno);
   exit:;
   }

   void
   GetStreamInfo(common::StreamInfo *info, error *err)
   {
      info->IsRemote = fd_is_remote(fd, err);
      ERROR_CHECK(err);
   exit:;
   }
};

typedef UnixPStream PlatformPStream;

#endif
} // end namespace

void
common::CreateStream(
   const char *filename,
   common::OpenMode mode,
   common::PStream **out,
   error *err
)
{
   common::Pointer<PlatformPStream> r;

   New(r.GetAddressOf(), err);
   ERROR_CHECK(err);

   r->open(filename, mode, err);
   ERROR_CHECK(err);

exit:
   if (ERROR_FAILED(err))
      r = nullptr;
   *out = r.Detach();
}

namespace {

struct StreamWrapper : public common::Stream
{
   common::Pointer<common::PStream> stream;
   uint64_t pos;

   StreamWrapper() : pos(0) {}

   uint64_t GetSize(error *err)     { return stream->GetSize(err); }
   uint64_t GetPosition(error *err) { return pos; }
   void Flush(error *err)           { stream->Flush(err); }

   void
   Seek(int64_t pos, int whence, error *err)
   {
      uint64_t fileSize = GetSize(err);
      ERROR_CHECK(err);

      switch (whence)
      {
      case SEEK_SET:
         break;
      case SEEK_CUR:
         Seek(this->pos + pos, SEEK_SET, err);
         return;
      case SEEK_END:
         Seek(fileSize + pos, SEEK_SET, err);
         return;
      default:
         ERROR_SET(err, unknown, "Bad seek whence");
      }

      // Set pos...
      //
      this->pos = MIN(fileSize, MAX(0LL, pos));
   exit:;
   }

   size_t
   Read(void *buf, size_t len, error *err)
   {
      auto r = stream->Read(buf, len, pos, err);
      ERROR_CHECK(err);
      if (r > 0)
         pos += r;
   exit:
      return r;
   }

   size_t
   Write(const void *buf, size_t len, error *err)
   {
      auto r = stream->Write(buf, len, pos, err);
      ERROR_CHECK(err);
      if (r > 0)
         pos += r;
   exit:
      return r;
   }

   void
   Truncate(uint64_t length, error *err)
   {
      stream->Truncate(length, err);
   }

   void
   GetStreamInfo(common::StreamInfo *info, error *err)
   {
      stream->GetStreamInfo(info, err);
   }

   void
   ToPStream(common::PStream **stream, error *err)
   {
      (*stream = this->stream.Get())->AddRef();
   }
};

} // end namespace

size_t
common::PStream::Write(const void *buf, size_t len, uint64_t off, error *err)
{
#if defined(_WINDOWS)
   ERROR_SET(err, win32, ERROR_ACCESS_DENIED);
#else
   ERROR_SET(err, errno, EACCES);
#endif
exit:
   return -1;
}

void
common::PStream::ToStream(common::Stream **out, error *err)
{
   common::Pointer<StreamWrapper> r;

   New(r.GetAddressOf(), err);
   ERROR_CHECK(err);

   r->stream = this;

exit:
   if (ERROR_FAILED(err))
      r = nullptr;
   *out = r.Detach();
}

namespace {

struct SubstreamWrapper : public common::PStream
{
   common::Pointer<common::PStream> baseStream;
   uint64_t pos;
   uint64_t len;

   uint64_t GetSize(error *err) { return len; }
   void Flush(error *err)       { baseStream->Flush(err); }
   void Truncate(uint64_t length, error *err)
   {
      if (len < length)
         ERROR_SET(err, unknown, "Can't truncate beyond substream bounds");
      len = length;
   exit:;
   }
   void GetStreamInfo(common::StreamInfo *info, error *err) {baseStream->GetStreamInfo(info, err); }

   void
   Translate(uint64_t &callerPos, size_t &callerLen, error *err)
   {
      callerLen = MIN(len - callerPos, callerLen);
      callerPos += pos;
   }

   size_t Read(void *buf, size_t len, uint64_t pos, error *err)
   {
      size_t r = 0;
      Translate(pos, len, err);
      ERROR_CHECK(err);
      r = baseStream->Read(buf, len, pos, err);
      ERROR_CHECK(err);
   exit:
      return r;
   }

   size_t Write(const void *buf, size_t len, uint64_t pos, error *err)
   {
      size_t r = 0;
      Translate(pos, len, err);
      ERROR_CHECK(err);
      r = baseStream->Write(buf, len, pos, err);
      ERROR_CHECK(err);
   exit:
      return r;
   }

   void
   ToStream(common::Stream **stream, error *err)
   {
      common::Pointer<common::Stream> wrapped;

      baseStream->ToStream(wrapped.GetAddressOf(), err);
      ERROR_CHECK(err);
      wrapped->Substream(this->pos, this->len, stream, err);
      ERROR_CHECK(err);
   exit:;
   }

   void Substream(uint64_t pos, uint64_t len, PStream **out, error *err)
   {
      if (pos > this->len || pos + len > this->len)
         ERROR_SET(err, unknown, "Substream out of bounds");
      baseStream->Substream(this->pos + pos, len, out, err);
      ERROR_CHECK(err);
   exit:;
   }
};

} // end namepsace

void
common::PStream::Substream(uint64_t pos, uint64_t len, PStream **out, error *err)
{
   common::Pointer<SubstreamWrapper> wrapped;
   uint64_t size = this->GetSize(err);

   ERROR_CHECK(err);

   if (pos > size || pos+len > size)
      ERROR_SET(err, unknown, "Substream out of bounds");

   common::New(wrapped, err);
   ERROR_CHECK(err);

   wrapped->baseStream = this;
   wrapped->pos = pos;
   wrapped->len = len;

exit:
   if (ERROR_FAILED(err))
      wrapped = nullptr;
   *out = wrapped.Detach();
}