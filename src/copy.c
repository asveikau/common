/*
 Copyright (C) 2020 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

//
// On Windows:
//    Call CopyFileW
// On Unix:
//    We try to use in-kernel I/O, one motivation for this is I want to
//    be ready for things like NFS server side copy.
//    Linux: use copy_file_range(2) or sendfile(2), falling back to
//    read(2) and write(2) for old kernels.
//       TODO: remove syscall stub if requiring glibc 2.27 or later
//       TODO: FreeBSD added copy_file_range(2) in June of 2019, not in a
//             release yet
//    Solaris: use sendfile(3C)
//    Other: read(2) and write(2)
//

#include <common/path.h>
#include <common/misc.h>
#include <common/size.h>

#if defined(_WINDOWS)
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

#if defined(_WINDOWS)
void
copy_file(const char *src, const char *dst, error *err)
{
   PWSTR srcW = NULL;
   PWSTR dstW = NULL;
   DWORD attrs = 0;

   srcW = ConvertToPwstr(src, err);
   ERROR_CHECK(err);

   dstW = ConvertToPwstr(dst, err);
   ERROR_CHECK(err);

   attrs = GetFileAttributes(dstW);
   if (attrs != INVALID_FILE_ATTRIBUTES)
   {
      DWORD badAttrs = (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_READONLY);
      if ((attrs&badAttrs))
         SetFileAttributes(dstW, attrs & ~badAttrs);
   }

   if (!CopyFile(srcW, dstW, FALSE))
      ERROR_SET(err, win32, GetLastError());

exit:
   free(srcW);
   free(dstW);
}
#elif !defined(__sun__)

static
void
copy_fds_readwrite(struct stat *stat, int infd, int outfd, error *err)
{
   char buf[4096];
   int r = 0;

   while ((r = read(infd, buf, sizeof(buf))) > 0)
   {
      char *dstbuf = buf;
      int n = r;

      while (n)
      {
         r = write(outfd, dstbuf, n);
         if (r < 0)
            ERROR_SET(err, errno, errno);
         if (!r)
            ERROR_SET(err, unknown, "Unespected zero write");
         dstbuf += r;
         n -= r;
      }
   }
   if (r < 0)
      ERROR_SET(err, errno, errno);
exit:;
}

#endif

#if defined(__linux__)
#include <sys/syscall.h>
#include <sys/types.h>
#define copy_file_range copy_file_range_local
// NB: Added to glibc 2.27 on 2018-02-01.
// Some day we can remove this.
static
loff_t
copy_file_range(
   int fd_in,
   loff_t *off_in,
   int fd_out,
   loff_t *off_out,
   size_t len,
   unsigned int flags
)
{
   return syscall(
      __NR_copy_file_range,
      fd_in,
      off_in,
      fd_out,
      off_out,
      len,
      flags
   );
}
#endif

#if defined(__linux__) || defined(__sun__)
#include <sys/sendfile.h>

static
ssize_t
copy_range(
   int infd,
   int outfd,
   uint64_t off,
   size_t len
)
{
   ssize_t r;
#if defined(__linux__)
   off_t readOff = off, writeOff = off;
   r = copy_file_range(infd, &readOff, outfd, &writeOff, len, 0);
   if (r < 0 && errno == ENOSYS)
      r = sendfile(outfd, infd, &readOff, len);
#else
   off_t offp = off;
   r = sendfile(outfd, infd, &offp, len);
#endif
   return r;
}

static
void
copy_fds(struct stat *stat, int infd, int outfd, error *err)
{
   uint64_t len = 0;
   off_t off = 0;

   len = stat->st_size;

   while (len)
   {
      ssize_t r = copy_range(infd, outfd, off, MIN(len, SSIZE_MAX));
#if defined(__linux__)
      // On Linux 2.6.0 through 2.6.32 [2010-02-24], sendfile(2) needs outfd to
      // be a socket.
      //
      if (r < 0 && errno == ENOTSOCK && !off)
      {
         copy_fds_readwrite(stat, infd, outfd, err);
         return;
      }
#endif
      if (r < 0)
         ERROR_SET(err, errno, errno);
      if (!r)
         ERROR_SET(err, unknown, "Unespected zero write");
      len -= r;
      off += r;
   }
exit:;
}
#else
#define copy_fds copy_fds_readwrite
#endif

#if !defined(_WINDOWS)
void
copy_file(const char *src, const char *dst, error *err)
{
   int infd = -1;
   int outfd = -1;

   struct stat stat;

   infd = open(src, O_RDONLY);
   if (infd < 0)
      ERROR_SET(err, errno, errno);

   if (fstat(infd, &stat))
      ERROR_SET(err, errno, errno);

   outfd = open(dst, O_CREAT|O_WRONLY|O_TRUNC, (stat.st_mode & 0777));
   if (outfd < 0)
      ERROR_SET(err, errno, errno);

   if (fchmod(outfd, (stat.st_mode & 0777)))
      ERROR_SET(err, errno, errno);

   copy_fds(&stat, infd, outfd, err);
   ERROR_CHECK(err);

exit:
   if (infd >= 0)
      close(infd);
   if (outfd >= 0)
      close(outfd);
}
#endif
