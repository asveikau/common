/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/thread.h>
#include <common/mutex.h>
#include <common/error.h>

#if defined(MUTEX_WINDOWS)
#include <windows.h>

void
create_thread(
   void *context,
   thread_proc fn,
   thread_id *id_out,
   error *err
)
{
   DWORD id = 0;
   id_out->Id = 0;
   id_out->Handle = NULL;
   if (!CreateThread(NULL, 0, fn, context, CREATE_SUSPENDED, &id))
      ERROR_SET(err, win32, GetLastError());
   id_out->Handle = OpenThread(
      SYNCHRONIZE | THREAD_QUERY_INFORMATION | THREAD_SUSPEND_RESUME,
      FALSE,
      id
   );
   if (!id_out->Handle)
      ERROR_SET(err, win32, GetLastError());
   id_out->Id = id;
   if (ResumeThread(id_out->Handle) == (DWORD)-1)
      ERROR_SET(err, win32, GetLastError());
exit:
   if (id_out->Handle && ERROR_FAILED(err))
   {
      CloseHandle(id_out->Handle);
      id_out->Handle = NULL;
   }
}

void
detach_thread(thread_id *id)
{
   if (id && id->Handle)
   {
      CloseHandle(id->Handle);
      id->Handle = NULL;
   }
}

void
join_thread(thread_id *id)
{
   if (id && id->Handle)
   {
      WaitForSingleObject(id->Handle, INFINITE);
      CloseHandle(id->Handle);
      id->Handle = NULL;
   }
}

bool
thread_is_self(thread_id *id)
{
   return thread_is_started(id) &&
          (id->Id == GetCurrentThreadId());
}

bool
thread_is_started(thread_id *id)
{
   return id && id->Handle;
}

void
thread_get_self(thread_id *id)
{
   id->Id = GetCurrentThreadId();
   id->Handle = GetCurrentThread();
}

#elif defined(MUTEX_PTHREAD)

void
create_thread(
   void *context,
   thread_proc fn,
   thread_id *id_out,
   error *err
)
{
   int r = 0;
   pthread_attr_t *attr = NULL;

   id_out->init = false;

   r = pthread_create(
      &id_out->th,
      attr,
      fn,
      context
   );
   if (r)
      ERROR_SET(err, errno, r);

   id_out->init = true;
exit:;
}

void
detach_thread(thread_id *id)
{
   if (id && id->init)
   {
      pthread_detach(id->th);
      id->init = false;
   }
}

void
join_thread(thread_id *id)
{
   if (id && id->init)
   {
      void *dummy = NULL;
      pthread_join(id->th, &dummy);
      id->init = false;
   }
}

bool
thread_is_self(thread_id *id)
{
   return thread_is_started(id) && !!pthread_equal(id->th, pthread_self());
}

bool
thread_is_started(thread_id *id)
{
   return id && id->init;
}

void
thread_get_self(thread_id *id)
{
   id->init = true;
   id->th = pthread_self();
}

#endif
