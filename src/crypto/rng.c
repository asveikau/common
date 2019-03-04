/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/crypto/rng.h>
#include <common/misc.h>

#if defined(_WINDOWS)

typedef BOOLEAN
(WINAPI *RtlGenRandomType)(
   PVOID,
   ULONG
);
static RtlGenRandomType
RtlGenRandom = NULL;

void
rng_init(
   struct rng_state **ret,
   error *err
)
{
   static const CHAR funcName[] = "SystemFunction036";

   *ret = NULL;

   if (!RtlGenRandom)
   {
      PWSTR modules[] =
      {
         L"cryptbase.dll",
         L"advapi32.dll",
         NULL
      };
      PWSTR *modName = modules;
      RtlGenRandomType r = NULL;

      while (*modName)
      {
         HMODULE mod = NULL;

         mod = LoadLibrary(*modName++);
         if (mod)
         {
            r = (RtlGenRandomType)GetProcAddress(mod, funcName);
            if (r)
               break;
            FreeLibrary(mod);
         }
      }

      if (!r)
         ERROR_SET(err, unknown, "Could not find RtlGenRandom");

      RtlGenRandom = r;
   }
exit:;
}

void
rng_generate(
   struct rng_state *st,
   void *buffer,
   size_t len, 
   error *err
)
{
   if (!RtlGenRandom)
      ERROR_SET(err, unknown, "rng_init not called");
   if (!RtlGenRandom(buffer, len))
      ERROR_SET(err, unknown, "RtlGenRandom failed");
exit:;
}

void
rng_close(struct rng_state *st)
{
}

#elif defined(__linux__)

//
// Linux has a syscall for randomness as of 3.17 (October 2014)
// It was not wrapped by glibc until 2.25 (February 2017) so we will
// invoke it via syscall(2) and fall back to /dev/urandom.
// 

#include <asm/unistd.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <common/lazy.h>

struct rng_state
{
   int fd;
   lazy_init_state lazy;
};
static struct rng_state singleton = {-1, {0}};

void
rng_init(
   struct rng_state **ret,
   error *err
)
{
   *ret = &singleton;
}

static void
open_random(void *ctx, error *err)
{
   struct rng_state *st = ctx;
   st->fd = open("/dev/urandom", O_RDONLY, 0);
   if (st->fd < 0)
      ERROR_SET(err, errno, errno);
exit:;
}

void
rng_generate(
   struct rng_state *st,
   void *buffer,
   size_t len, 
   error *err
)
{
   if (!len)
      return;

#if defined(__NR_getrandom)
   if (st->fd >= 0)
      goto skip;
   for (;;)
   {
      if (!len)
         goto exit;

      long r = syscall(__NR_getrandom, buffer, len, 0);
      if (r > 0)
      {
         r = MIN(len, r);
         buffer = (char*)buffer + r;
         len -= r;
      }
      else if (!r)
         ERROR_SET(err, errno, ENOSYS);
      else if (errno != ENOSYS)
         ERROR_SET(err, errno, errno);
      else
         break;
   }
skip:
#endif
   lazy_init(&st->lazy, open_random, st, err);
   ERROR_CHECK(err);

   for (;;)
   {
      int r = read(st->fd, buffer, len);
      if (r < 0)
         ERROR_SET(err, errno, errno);
      if (!r)
         ERROR_SET(err, unknown, "EOF reading from random device");
      buffer = (char*)buffer + r;
      len -= r;
      if (!len)
         break;
   }
exit:;
}

void
rng_close(struct rng_state *st)
{
}

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)

//
// In recent *BSDs arc4random_buf is backed by a kernel syscall, but
// documentation says not to use the syscall directly.
//
// In OS X (and probably older BSD) arc4random_buf is an open("/dev/random")
// with bad failure behaviors.
// Vide: https://twitter.com/FiloSottile/status/643578513602252800
//

#include <stdlib.h>

void
rng_init(
   struct rng_state **ret,
   error *err
)
{
   *ret = NULL;
}

void
rng_generate(
   struct rng_state *st,
   void *buffer,
   size_t len, 
   error *err
)
{
   arc4random_buf(buffer, len);
}

void
rng_close(struct rng_state *st)
{
}

#else

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

struct rng_state
{
   int fd;
};

void
rng_init(struct rng_state **ret, error *err)
{
   struct rng_state *st = NULL;

   st = malloc(sizeof(*st));
   if (!st) ERROR_SET(err, nomem);

   st->fd = open("/dev/random", O_RDONLY, 0);
   if (st->fd < 0)
      ERROR_SET(err, errno, errno);

   *ret = st;
   st = NULL;
exit:
   rng_close(st);
}

void
rng_generate(
   struct rng_state *st,
   void *buffer,
   size_t len, 
   error *err
)
{
   for (;;)
   {
      int r = read(st->fd, buffer, len);
      if (r < 0)
         ERROR_SET(err, errno, errno);
      if (!r)
         ERROR_SET(err, unknown, "EOF reading from random device");
      buffer = (char*)buffer + r;
      len -= r;
      if (!len)
         break;
   }
exit:;
}

void
rng_close(struct rng_state *st)
{
   if (st)
   {
      if (st->fd >= 0)
         close(st->fd);
      free(st);
   }
}

#endif
