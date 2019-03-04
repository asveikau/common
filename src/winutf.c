/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/misc.h>

PSTR
ConvertToPstr(PCWSTR str, error *err)
{
   DWORD wideLen = wcslen(str) + 1;
   DWORD utf8Len = wideLen * 4;
   PSTR r = malloc(utf8Len);
   if (r)
   {
      int cch = WideCharToMultiByte(
         CP_UTF8,
         0,
         str,
         wideLen,
         r,
         utf8Len,
         NULL,
         NULL 
      );
      if (!cch)
      {
         ERROR_SET(err, win32, GetLastError());
      }
   }
   else
   {
      ERROR_SET(err, nomem);
   }
exit:
   if (ERROR_FAILED(err)) { free(r); r = NULL; }
   return r;
}

PWSTR
ConvertToPwstr(PCSTR str, error *err)
{
   DWORD utf8Len = strlen(str) + 1;
   DWORD wideLen = utf8Len * sizeof(WCHAR);
   PWSTR r = malloc(wideLen);
   if (r)
   {
      int cch = MultiByteToWideChar(
         CP_UTF8,
         0,
         str,
         utf8Len,
         r,
         wideLen / sizeof(WCHAR)
      );
      if (!cch)
      {
         ERROR_SET(err, win32, GetLastError());
      }
   }
   else
   {
      ERROR_SET(err, nomem);
   }
exit:
   if (ERROR_FAILED(err)) { free(r); r = NULL; }
   return r;
}

