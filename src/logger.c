/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/logger.h>
#include <common/buffer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WINDOWS)
#include <windows.h>
#else
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#endif

#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE 4096
#endif

void
log_printf(const char *fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);
   log_vprintf(fmt, ap);
   va_end(ap);
}

// XXX this global data structure is not assumed to be atomic or particularly
// efficient.

typedef struct
{
   logger_callback_fn fn;
   void *context;
   int id;
} logger_registration;

static buffer registered_loggers = {0};

#if !defined(_WINDOWS)
#if defined(__linux__)
#include <sys/syscall.h>
#endif
#if defined(__NetBSD__)
#include <lwp.h>
#endif
static long long
get_thread_id()
{
#if defined(__linux__)
   return syscall(SYS_gettid);
#elif defined(__APPLE__)
   uint64_t r = 0;
   pthread_threadid_np(pthread_self(), &r);
   return r;
#elif defined(__FreeBSD__)
   return pthread_getthreadid_np();
#elif defined(__NetBSD__)
   return _lwp_self();
#else
   return (long long)pthread_self();
#endif
}
#endif

void
log_vprintf(const char *fmt, va_list ap)
{
   char stack_buf[LOG_BUFFER_SIZE];
   char *heap_buf = NULL;
   char *log_buf = stack_buf;
   int stack_buf_consumed = 0;
   va_list ap2;
   int r = 0;
   logger_registration *p = NULL, *q = NULL;
   int nlpad = fmt && *fmt && (fmt[strlen(fmt)-1] != '\n');

   // Anybody listening for messages?
   //
   if (!BUFFER_NBYTES(&registered_loggers))
      return;

#if defined(_WINDOWS)
   SYSTEMTIME time;
   GetLocalTime(&time);

   stack_buf_consumed = snprintf(
      stack_buf, sizeof(stack_buf),
      "[P:%d T:%d %.4d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d] ",
      GetCurrentProcessId(),
      GetCurrentThreadId(),
      time.wYear, time.wMonth, time.wDay,
      time.wHour, time.wMinute, time.wSecond, time.wMilliseconds
   ); 
#else
   struct timeval tv;
   struct tm tm;
   gettimeofday(&tv, NULL); 
   localtime_r(&tv.tv_sec, &tm);

   stack_buf_consumed = snprintf(
      stack_buf, sizeof(stack_buf),
      "[P:%d T:%lld %.4d-%.2d-%.2d %.2d:%.2d:%.2d.%.3d] ",
      getpid(),
      get_thread_id(),
      1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday,
      tm.tm_hour, tm.tm_min, tm.tm_sec,
      (int)(tv.tv_usec / 1000)
   );
#endif
   log_buf += stack_buf_consumed;

   va_copy(ap2, ap);

   r = vsnprintf(
      log_buf,
      sizeof(stack_buf) - stack_buf_consumed - nlpad,
      fmt,
      ap
   );
   if (r >= sizeof(stack_buf) - stack_buf_consumed - nlpad)
   {
      heap_buf = malloc(stack_buf_consumed + r + 1 + nlpad);
      if (heap_buf)
      {
         memcpy(heap_buf, stack_buf, stack_buf_consumed);
         memset(heap_buf + stack_buf_consumed, 0, r+1+nlpad);
         vsnprintf(heap_buf + stack_buf_consumed, r+1, fmt, ap2);
         log_buf = heap_buf;
      }
      else
      {
         static const char msg[] =
           "<Message dropped due to malloc failure>";
         memcpy(stack_buf, msg, sizeof(msg));
         nlpad = 0;
      }
   } 
   else
   {
      log_buf = stack_buf;
   }

   va_end(ap2);

   if (nlpad)
      memcpy(log_buf + stack_buf_consumed + r, "\n", 2);

   for (p = BUFFER_PTR(&registered_loggers),
        q = p + BUFFER_NMEMB(&registered_loggers, *q); p < q; ++p)
   {
      p->fn(p->context, log_buf);
   }

   free(heap_buf);
}

int
log_register_callback(logger_callback_fn fn, void *context)
{
   logger_registration *p = NULL;
   static int next_id = 0;

   if (!(p = buffer_alloc(&registered_loggers, sizeof(*p))))
      return -1;

   p->fn = fn;
   p->context = context;
   p->id = next_id++;

   return p->id;
}

void
log_unregister_callback(int id)
{
   logger_registration *p = NULL, *q = NULL;

   for (p = BUFFER_PTR(&registered_loggers),
        q = p + BUFFER_NMEMB(&registered_loggers, *q); p < q; ++p)
   {
      if (p->id == id)
      {
         int i = (char*)p - (char*)BUFFER_PTR(&registered_loggers);
         buffer_remove(
            &registered_loggers,
            i,
            sizeof(*p)
         );
         break;
      }
   }
}
