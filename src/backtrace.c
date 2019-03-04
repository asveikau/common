/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/backtrace.h>
#include <common/misc.h>

#include <stdio.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

static int
describe_offset(void *base, void *addr, char *buf, int bufsz)
{
   if (base != addr)
   {
      snprintf(buf, bufsz, "+%x", (int)((char*)addr - (char*)base));
      return strlen(buf);
   }
   return 0;
}

#if defined(_WINDOWS)
#include <windows.h>

size_t
backtrace(void **addrs, size_t naddrs)
{
   return RtlCaptureStackBackTrace(1, naddrs, addrs, NULL);  
}

void
describe_symbol(void *addr, char *buf, int bufsz)
{
   HMODULE mod = NULL;
   int r = 0;

   if (bufsz < 2)
      goto exit;

   snprintf(buf, bufsz - 1, "%p", addr);
   r = strlen(buf);
   bufsz -= r;
   buf += r;

   GetModuleHandleEx(
      GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS  
         | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
      addr,
      &mod
   );
   if (mod)
   {
      WCHAR name[MAX_PATH];
      if (GetModuleFileName(mod, name, ARRAY_SIZE(name)))
      {
         PCWSTR slash = wcsrchr(name, L'\\');
         slash = slash ? (slash+1) : name;
         if (bufsz < 2) goto exit;
         --bufsz;
         *buf++ = ' ';
         if (bufsz < 2) goto exit;
         --bufsz;
         *buf++ = '(';
         if (bufsz < 2) goto exit;
         r = WideCharToMultiByte(
            CP_UTF8,
            0,
            slash,
            wcslen(slash),
            buf,
            bufsz - 1,
            NULL,
            NULL
         ); 
         if (r <= 0) goto exit;
         bufsz -= r;
         buf += r;

         if (!bufsz) goto exit;
         r = describe_offset(mod, addr, buf, bufsz-1);
         bufsz -= r;
         buf += r;

         if (bufsz < 2) goto exit;
         --bufsz;
         *buf++ = ')';
      }
   }
exit:
   if (bufsz)
      *buf = 0;
}

#else

#include <dlfcn.h>

void
describe_symbol(void *addr, char *buf, int bufsz)
{
   int r = 0;
   Dl_info info;

   snprintf(buf, bufsz, "%p", addr);
   r = strlen(buf);
   bufsz -= r;
   buf += r;

   if (dladdr(addr, &info))
   {
      const char *file = strrchr(info.dli_fname, '/');
      file = file ? file + 1 : info.dli_fname;

      snprintf(buf, bufsz, " (%s", file);
      r = strlen(buf);
      bufsz -= r;
      buf += r;
      if (info.dli_sname)
      {
         snprintf(buf, bufsz, "!%s", info.dli_sname);
         r = strlen(buf);
         bufsz -= r;
         buf += r;

         r = describe_offset(info.dli_saddr, addr, buf, bufsz);
         bufsz -= r;
         buf += r;
      }
      else
      {
         r = describe_offset(info.dli_fbase, addr, buf, bufsz);
         bufsz -= r;
         buf += r;
      }
      if (bufsz > 1)
      {
         --bufsz;
         *buf++ = ')';
      }
   } 

   if (bufsz)
      *buf = 0;
}

#endif
