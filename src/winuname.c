/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <windows.h>
#include <stdio.h>
#include <string.h>

#include <common/uname.h>

#pragma comment(lib, "version.lib")

int
uname(struct utsname *buf)
{
   PBYTE KSharedData = (PBYTE)0x7FFE0000;
   ULONG Major = *(PULONG)(KSharedData + 0x26c);
   ULONG Minor = *(PULONG)(KSharedData + 0x270);
   ULONG Build = 0;

   // KSharedData build number introduced in Win10...
   if (Major >= 10)
      Build = *(PULONG)(KSharedData + 0x260);
   // If we're not on Win8.1 we can use the legit API...
   else if (Major < 6 || (Major == 6 && Minor < 3))
   {
      OSVERSIONINFO ver = {0};
      ver.dwOSVersionInfoSize = sizeof(ver);
      if (GetVersionEx(&ver))
      {
         Build = ver.dwBuildNumber;
      }
   }
   // Win8.1 and higher lies to us, figure out product version of ntdll.
   else
   {
      static const WCHAR ntdllName[] = L"ntdll.dll";
      DWORD len = GetFileVersionInfoSize(ntdllName, NULL);
      if (len)
      {
         PVOID verInfo = _alloca(len);
         VS_FIXEDFILEINFO *info = NULL;
         UINT len2 = 0;

         if (GetFileVersionInfo(ntdllName, 0, len, verInfo) &&
             VerQueryValue(verInfo, L"\\", (PVOID*)&info, &len2))
         {
            LARGE_INTEGER li;

            li.HighPart = info->dwProductVersionMS;
            li.LowPart = info->dwProductVersionLS;

            Build = ((li.QuadPart >> 16) & 0xffff);
         }
      }
   }

   memset(buf, 0, sizeof(buf));

   strcpy(buf->sysname, "Windows");

   snprintf(
      buf->release, sizeof(buf->release), "%u.%u.%u", Major, Minor, Build
   );

#if defined(_M_IX86)
   strcpy(buf->machine, "x86");
#elif defined(_M_AMD64)
   strcpy(buf->machine, "amd64"); 
#elif defiend(_M_ARM)
   strcpy(buf->machine, "arm"); 
#endif

   return 0;
}




