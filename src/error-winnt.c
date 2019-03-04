/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/error.h>
#include <common/lazy.h>

#include <winnt.h>
#include <winternl.h>

void
error_set_ntstatus(error *err, NTSTATUS status)
{
   HRESULT hr = S_OK;
   if (!NT_SUCCESS(status))
   {
      DWORD win32 = RtlNtStatusToDosError(status);
      if (win32 != ERROR_MR_MID_NOT_FOUND)
      {
         error_set_win32(err, win32);
         return;
      }
      hr = (status | FACILITY_NT_BIT);
   }
   error_set_hresult(err, hr);
}

static void
GetRtlFunc(void *context, error *err)
{
   ULONG (WINAPI **Fn)(NTSTATUS) = context;
   HMODULE hm = GetModuleHandle(L"ntdll.dll");
   if (hm)
      *Fn = (PVOID)GetProcAddress(hm, "RtlNtStatusToDosError");
}

ULONG WINAPI
RtlNtStatusToDosError(NTSTATUS status)
{
   static ULONG (WINAPI *Fn)(NTSTATUS) = NULL;
   static lazy_init_state init = {0};

   lazy_init(
      &init,
      GetRtlFunc,
      &Fn,
      NULL
   );

   if (Fn)
      return Fn(status);

   return ERROR_MR_MID_NOT_FOUND;
}
