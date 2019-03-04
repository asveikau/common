/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/time.h>

#if defined(_WINDOWS)

#include <windows.h>

uint64_t
get_current_time()
{
   union
   {
      FILETIME fileTime;
      ULARGE_INTEGER li;
   } ts = {0};

   GetSystemTimeAsFileTime(&ts.fileTime);

   return ts.li.QuadPart / 10000000ULL -
          11644473600ULL;
}

#else

#include <time.h>

uint64_t
get_current_time()
{
   return time(NULL);
}

#endif
