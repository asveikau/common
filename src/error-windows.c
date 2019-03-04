/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/error.h>
#include <common/size.h>

static const char *
hresult_get_string(error *err)
{
   HRESULT hr = err->code;
   DWORD r = 0;
   PWSTR wideBuffer = NULL;
   char *heapBuffer = NULL;

   switch (hr)
   {
   case E_OUTOFMEMORY:
   case __HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY):
      return "Out of memory";
   }

   if (err->context)
      return err->context;

   r = FormatMessage(
      (FORMAT_MESSAGE_ALLOCATE_BUFFER |
       FORMAT_MESSAGE_FROM_SYSTEM |
       FORMAT_MESSAGE_IGNORE_INSERTS),
      NULL,
      hr,
      0,
      (PVOID)&wideBuffer,
      0,
      NULL
   );

   if (r > 0)
   {
      size_t n = 0;

      while (r && (wideBuffer[r-1] == L'\r' || wideBuffer[r-1] == L'\n'))
         wideBuffer[--r] = 0;

      if (r &&
          !size_add(r, 1, &n) &&
          !size_mult(4, n, &n))
      {
         heapBuffer = malloc(n);
      }

      if (heapBuffer)
      {
         int r2 = WideCharToMultiByte(
            CP_UTF8,
            0,
            wideBuffer,
            r + 1,
            heapBuffer,
            n,
            NULL,
            NULL
         );
         if (r2 > 0)
         {
            err->context = heapBuffer;
            err->free_fn = free;
            heapBuffer = NULL;
         }
      }
   }

   free(heapBuffer);
   if (wideBuffer)
      HeapFree(GetProcessHeap(), 0, wideBuffer);

   return err->context;
}

void
error_set_hresult(error *err, HRESULT hr)
{
   error_clear(err);
   if (FAILED(hr))
   {
      err->source = ERROR_SRC_COM;
      err->code = hr;
      err->get_string = hresult_get_string;
   }
   else
   {
      error_set_unknown(err, "Failure case, but no error code?");
   }
}

void
error_set_win32(error *err, DWORD code)
{
   HRESULT hr = 
      ((code & 0xffff0000U) ? code : HRESULT_FROM_WIN32(code));
   error_set_hresult(err, hr);
}

