/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/path.h>
#include <common/misc.h>

#include <stdlib.h>
#include <string.h>

#if !defined(__GLIBC__)
static const char *
argv0 = NULL;
#endif

#if !defined(_WINDOWS)
#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#define PATH_BUFFER_SIZE (PATH_MAX + 1)
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || \
    defined(__APPLE__)
#define HAVE_GETPROGNAME
#endif

#if defined(__linux__)
#define PROC_PATH "/proc/self/exe"
#endif

#if defined(__FreeBSD__) && 0
#define PROC_PATH "/proc/curproc/file"
#endif

#if defined(__NetBSD__)
#define PROC_PATH "/proc/curproc/exe"
#endif

#if defined(__APPLE__) || \
    defined(_WINDOWS) || \
    defined(__FreeBSD__) || \
    defined(PROC_PATH)
#define HAVE_LAZY_CHECK
#include <common/lazy.h>

static void
lazy_runtime_check(void *context, error *err);

static const char *
lazy_get_exe(error *err)
{
   static lazy_init_state lazy = {0};
   static const char *p = NULL;
   lazy_init(&lazy, lazy_runtime_check, &p, err);
   return p;
}
#endif

const char *
get_program_path(error *err)
{
   const char *r = NULL;

#if defined(HAVE_LAZY_CHECK)
   r = lazy_get_exe(err);
   if (ERROR_FAILED(err))
   {
      r = NULL;
      error_clear(err);
   }
   if (r)
      goto exit;
#endif

#if defined(__sun__)
   if ((r = getexecname()))
      goto exit;
#endif

#if defined(__GLIBC__)
   if ((r = program_invocation_name))
      goto exit;
#else
   if ((r = argv0))
      goto exit;
#endif

#if defined(HAVE_GETPROGNAME)
   r = getprogname();
   if (r)
      goto exit;
#endif

exit:
   return r;
}

#if defined(__APPLE__)

#include <mach-o/dyld.h>

static void
lazy_runtime_check(void *context, error *err)
{
   static char lazy_buffer[PATH_BUFFER_SIZE];
   const char **p = context;
   uint32_t sz = sizeof(lazy_buffer);
   if (_NSGetExecutablePath(lazy_buffer, &sz))
      ERROR_SET(err, unknown, "Couldn't get executable path");
   *p = lazy_buffer;
exit:;
}

#endif

#if defined(__FreeBSD__)

#include <sys/sysctl.h>

static void
lazy_runtime_check(void *context, error *err)
{
   static char lazy_buffer[PATH_BUFFER_SIZE];
   size_t len = sizeof(lazy_buffer);
   const char **p = context;

   static const int name[] = 
   {
      CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1
   };

   if (sysctl(name, ARRAY_SIZE(name), lazy_buffer, &len, NULL, 0) < 0)
      ERROR_SET(err, errno, errno); 

   *p = lazy_buffer;
exit:;
}

#endif

#if defined(PROC_PATH)

#include <unistd.h>

static void
lazy_runtime_check(void *context, error *err)
{
   static char lazy_buffer[PATH_BUFFER_SIZE];
   const char **p = context;
   int r = readlink(PROC_PATH, lazy_buffer, sizeof(lazy_buffer));
   if (r < 0)
      ERROR_SET(err, unknown, "readlink failed on " PROC_PATH);
   *p = lazy_buffer;
exit:;
}

#endif

#if defined(_WINDOWS)

static void
lazy_runtime_check(void *context, error *err)
{
   WCHAR buffer[4096];
   const char **p = context;

   if (!GetModuleFileName(NULL, buffer, ARRAY_SIZE(buffer)))
      ERROR_SET(err, win32, GetLastError());

   *p = ConvertToPstr(buffer, err);
exit:;
}

#endif

void
libcommon_set_argv0(const char *str)
{
#if !defined(__GLIBC__)
   argv0 = str;
#endif
}
