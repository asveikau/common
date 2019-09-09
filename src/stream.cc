/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/c++/stream.h>
#include <common/path.h>
#include <errno.h>
#include <new>

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#define HAVE_FILENO
#endif

#if defined(_WINDOWS)
#include <windows.h>
#include <io.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#include <common/misc.h>

using namespace common;

int
common::Stream::Write(const void *buf, int len, error *err)
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
common::StreamBase::Truncate(uint64_t length, error *err)
{
#if defined(_WINDOWS)
   ERROR_SET(err, win32, ERROR_ACCESS_DENIED);
#else
   ERROR_SET(err, errno, EACCES);
#endif
exit:;
}

namespace {

class FileStream : public Stream
{
   FILE *f_;

public:

   FileStream(FILE *f) : f_(f) {}

   ~FileStream()
   {
      fclose(f_);
   }

#if _WINDOWS
   HANDLE GetHandle() { return (HANDLE)_get_osfhandle(_fileno(f_)); }
   HANDLE GetFd()     { return GetHandle(); }
#elif defined(HAVE_FILENO)
   int GetFd()        { return fileno(f_); }
#else
   int GetFd()        { return -1; }
#endif

   uint64_t GetSize(error *err)
   {
      uint64_t r = 0;

#if _WINDOWS
      LARGE_INTEGER li = {0};

      if (!GetFileSizeEx(GetHandle(), &li))
         ERROR_SET(err, win32, GetLastError());

      r = li.QuadPart;
#elif defined(HAVE_FILENO)
      struct stat statbuf;
      if (fstat(GetFd(), &statbuf))
         ERROR_SET(err, errno, errno);
      r = statbuf.st_size;
#else
      auto oldPos = GetPosition(err);
      ERROR_CHECK(err);

      Seek(0, SEEK_END, err);
      ERROR_CHECK(err);

      r = GetPosition(err);
      ERROR_CHECK(err);

      Seek(oldPos, SEEK_SET, err);
      ERROR_CHECK(err);
#endif

   exit:
      return r;
   }

   uint64_t GetPosition(error *err)
   {
      auto r = ftello(f_);
      if (r < 0)
         ERROR_SET(err, errno, errno);
   exit:
      return r;
   }

   void Seek(int64_t pos, int whence, error *err)
   {
      if (fseeko(f_, pos, whence))
         ERROR_SET(err, errno, errno);
   exit:;
   }

   int Read(void *buf, int len, error *err)
   {
      if (!len)
         return 0;
      int r = fread(buf, 1, len, f_);
      if (!r && ferror(f_))
      {
         r = -1;
         ERROR_SET(err, errno, errno);
      }
   exit:
      return r;
   }

   int Write(const void *buf, int len, error *err)
   {
      int r = fwrite(buf, 1, len, f_);
      if (r != len)
         ERROR_SET(err, errno, errno);
   exit:
      return r;
   }

   void Flush(error *err)
   {
      if (fflush(f_))
         ERROR_SET(err, errno, errno);
   exit:;
   }

   void Truncate(uint64_t length, error *err)
   {
#if defined(_WINDOWS)
      uint64_t oldPos = GetPosition(err);
      ERROR_CHECK(err);
      Seek(length, SEEK_SET, err);
      ERROR_CHECK(err);
      if (!SetEndOfFile(GetHandle()))
         ERROR_SET(err, win32, GetLastError());
      Seek(oldPos, SEEK_SET, err);
      ERROR_CHECK(err);
#elif defined(HAVE_FILENO)
      if (ftruncate(GetFd(), length))
         ERROR_SET(err, errno, errno);
#else
      ERROR_SET(err, errno, ENOSYS);
#endif
   exit:;
   }

   void GetStreamInfo(StreamInfo *info, error *err)
   {
      auto fd = GetFd();
#if !defined(_WINDOWS)
      if (fd != -1)
#endif
      {
         info->IsRemote = fd_is_remote(fd, err);
      }
   }
};

} // namespace

void
common::CreateStream(FILE *f, Stream **out, error *err)
{
   try
   {
      *out = new FileStream(f); 
   }
   catch (std::bad_alloc)
   {
      ERROR_SET(err, nomem);
   }
exit:;
}

#if defined(_WINDOWS)

#if 0 // Naive windows implementation that uses _wfopen()

static FILE *
win_fopen(PCWSTR name16, const char *mode)
{   
   FILE *f = nullptr;
   PWSTR mode16 = nullptr;
   error err;

   // Force binary mode if it does not appear.
   //
   if (!strpbrk(mode, "tb"))
   {
      size_t l = strlen(mode);
      char *mode2 = (char*)alloca(l+2);
      snprintf(mode2, l+2, "%sb", mode);
      mode16 = ConvertToPwstr(mode2, &err);
   }
   else
   {
      mode16 = ConvertToPwstr(mode, &err);
   }
   ERROR_CHECK(&err);

   f = _wfopen(name16, mode16);
exit:
   free(mode16);
   return f;
}

#else // Let's get rid of this stupid concept of sharing flags, yes?

static FILE *
win_fopen(PCWSTR name16, const char *mode, DWORD dispositionOverride = 0)
{
   HANDLE hFile = INVALID_HANDLE_VALUE;
   DWORD desiredAccess = 0;
   DWORD disposition = 0;
   DWORD flagsAndAttrs = FILE_ATTRIBUTE_NORMAL;
   int fd = -1;
   int fdFlags = 0;
   FILE *f = nullptr;
   error err;

   switch (mode ? *mode : 0)
   {
   case 'r':
      desiredAccess = GENERIC_READ;
      disposition = OPEN_EXISTING;
      if (mode[1] == '+')
         desiredAccess |= GENERIC_WRITE;
      else
         fdFlags = _O_RDONLY;
      break;
   case 'w':
      desiredAccess = GENERIC_WRITE;
      disposition = CREATE_ALWAYS;
      if (mode[1] == '+')
         desiredAccess |= GENERIC_READ;
      break;
   case 'a':
      desiredAccess = FILE_APPEND_DATA;
      disposition = CREATE_ALWAYS;
      fdFlags = _O_APPEND;
      if (mode[1] == '+')
         desiredAccess |= GENERIC_READ;
      break;
   default:
      ERROR_SET(&err, unknown, "Invalid mode");
   }

   if (dispositionOverride)
      disposition = dispositionOverride;

   if (disposition == CREATE_ALWAYS)
   {
      DWORD attrs = GetFileAttributes(name16);
      const DWORD badFlags = (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY);
      if (attrs != INVALID_FILE_ATTRIBUTES && (attrs&badFlags))
         SetFileAttributes(name16, attrs & ~badFlags);
   }

   hFile = CreateFile(
      name16,
      desiredAccess,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      nullptr,
      disposition,
      flagsAndAttrs,
      nullptr
   );
   if (hFile == INVALID_HANDLE_VALUE)
      ERROR_SET(&err, win32, GetLastError());

   if ((fd = _open_osfhandle((intptr_t)hFile, fdFlags)) < 0)
      ERROR_SET(&err, unknown, "_open_osfhandle failed");

   hFile = INVALID_HANDLE_VALUE;

   f = _fdopen(fd, mode);
   if (!f)
      ERROR_SET(&err, errno, errno);

   fd = -1;
exit:
   if (hFile != INVALID_HANDLE_VALUE)
      CloseHandle(hFile);
   if (fd >= 0)
      _close(fd);
   if (err.source && err.source != ERROR_SRC_ERRNO)
      errno = EINVAL;
   return f;
}

#endif

static FILE *
win_fopen(const char *name, const char *mode, DWORD dispositionOverride = 0)
{
   FILE *f = nullptr;
   error err;

   PWSTR name16 = nullptr;

   name16 = ConvertToPwstr(name, &err);
   ERROR_CHECK(&err);

   f = win_fopen(name16, mode, dispositionOverride);
exit:
   free(name16);
   return f;
}

#undef fopen
#define fopen win_fopen

#endif

static void
SetupStream(FILE *f)
{
   if (BUFSIZ < 8192)
      setvbuf(f, nullptr, _IOFBF, 8192);
}

void
common::CreateStream(
   const char *filename,
   const char *mode, 
   Stream **out,
   error *err
)
{
   FILE *f = nullptr;

   f = fopen(filename, mode);
   if (!f)
      ERROR_SET(err, errno, errno);

   SetupStream(f);

   CreateStream(f, out, err);
   ERROR_CHECK(err);

   f = nullptr;
exit:
   if (f) fclose(f);
}

void
common::CreateStreamReadWriteCreate(
   const char *filename,
   Stream **out,
   error *err
)
{
   FILE *f = nullptr;

#if _WINDOWS
   f = fopen(filename, "r+", OPEN_ALWAYS);
#else
   int fd;

   fd = open(filename, O_RDWR | O_CREAT, 0666);
   if (fd < 0)
      ERROR_SET(err, errno, errno);

   f = fdopen(fd, "r+");
   if (f)
      fd = -1;
#endif

   if (!f)
      ERROR_SET(err, errno, errno);

   SetupStream(f);

   CreateStream(f, out, err);
   ERROR_CHECK(err);

   f = nullptr;
exit:
   if (f) fclose(f);
#ifndef _WINDOWS
   if (fd >= 0)
      close(fd);
#endif
}

void
common::CreateStream(
   const char *filename,
   OpenMode mode,
   Stream **out,
   error *err
)
{
   const char *modeString = nullptr;

   switch (mode)
   {
   case ReadWriteCreate:
      CreateStreamReadWriteCreate(filename, out, err);
      break;
   case ReadOnly:
      modeString = "r";
      break;
   case ReadWrite:
      modeString = "w";
      break;
   }

   if (!modeString)
      ERROR_SET(err, unknown, "invalid mode");

   CreateStream(filename, modeString, out, err);
exit:;
}
