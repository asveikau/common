/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/time.h>

#if defined (_WINDOWS)
#include <windows.h>
#else
#include <sys/time.h>
#include <time.h>
#endif

#if defined(XP_SUPPORT)

#include <common/lazy.h>

static void
find_func(PVOID param, error *err)
{
   PVOID *pp = param;
   HMODULE mod = GetModuleHandle(L"kernelbase.dll");
   if (!mod)
      mod = GetModuleHandle(L"kernel32.dll");
   if (mod)
      *pp = GetProcAddress(mod, "GetTickCount64");
}

#define GetTickCount64 GetTickCountWrapper

static ULONGLONG
GetTickCount64(void)
{
   static lazy_init_state state = {0};
   static ULONGLONG (WINAPI *fn)(void) = NULL;
   error err;

   memset(&err, 0, sizeof(err));
   lazy_init(&state, find_func, &fn, &err);
   error_clear(&err);

   if (fn)
      return fn();
   else
      return GetTickCount();
}

#endif

uint64_t
get_monotonic_time_millis()
{
#if defined (_WINDOWS)
   return GetTickCount64();
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
   struct timespec ts = {0};
   clock_gettime(CLOCK_MONOTONIC, &ts);
   return ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000;
#else
   // XXX not monotonic
   struct timeval tv = {0};
   gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000ULL + tv.tv_usec / 1000; 
#endif
}
