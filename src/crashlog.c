/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/backtrace.h>
#include <common/logger.h>
#include <common/misc.h>

#include <stdio.h>

static void (*on_crash)(void*) = NULL;
static void *on_crash_ctx = NULL;

#define BT_DEPTH 16

#if defined(_WINDOWS)

#include <windows.h>

static PCSTR
GetAccessMode(DWORD flags)
{
   switch (flags)
   {
   case 0: return "read";
   case 1: return "write";
   case 8: return "execute";
   }
   return "<invalid>";
}

LONG CALLBACK
SehCallback(PEXCEPTION_POINTERS Pointers)
{
   const char *event = NULL;
   char buffer[8192] = {0};

   switch (Pointers->ExceptionRecord->ExceptionCode)
   {
   case EXCEPTION_ACCESS_VIOLATION:
      event = "Access violation";
      snprintf(
         buffer,
         sizeof(buffer),
         "Tried to %s %p", 
         GetAccessMode(Pointers->ExceptionRecord->ExceptionInformation[0]),
         (PVOID)Pointers->ExceptionRecord->ExceptionInformation[1]
      );
      break;
   case EXCEPTION_IN_PAGE_ERROR:
      event = "Non-serviceable page fault";
      snprintf(
         buffer,
         sizeof(buffer),
         "Tried to %s %p; failed with %.8x", 
         GetAccessMode(Pointers->ExceptionRecord->ExceptionInformation[0]),
         (PVOID)Pointers->ExceptionRecord->ExceptionInformation[1],
         (NTSTATUS)Pointers->ExceptionRecord->ExceptionInformation[2]
      );
      break;
   case EXCEPTION_FLT_DIVIDE_BY_ZERO:
   case EXCEPTION_INT_DIVIDE_BY_ZERO:
      event = "Divide by zero";
      break;
   case EXCEPTION_ILLEGAL_INSTRUCTION:
      event = "Illegal instruction";
      break;
   case EXCEPTION_STACK_OVERFLOW:
      event = "Stack overflow";
      break;
   }

   if (event)
   {
      PVOID addr = Pointers->ExceptionRecord->ExceptionAddress;

      char *p = buffer;
      int n = sizeof(buffer);
      int r = 0;
      const char *details = (*p ? p : NULL);
      const char *msg = NULL;
      const char *stackString = "";

      if (details)
      {
         int m = strlen(buffer) + 1;
         n -= m;
         p += m;
      }

      msg = p;

      r = snprintf(p, n, "%s at ", event);
      if (n >= r)
      {
         p += r;
         n -= r;

         describe_symbol(addr, p, n);
         r = strlen(p);
         p += r;
         n -= r;
      }
      if (n > 1)
      {
         *p++ = 0;
         --n;
         *p = 0;
      }

      if (n)
      {
         PVOID stack[BT_DEPTH], *ps = stack;

         size_t nStack = backtrace(stack, ARRAY_SIZE(stack));

         stackString = p;

         while (n && nStack--)
         {
            if (n >= 1)
            {
               *p++ = '\n';
               --n;
               *p = 0;
            }
            describe_symbol(*ps++, p, n);
            int m = strlen(p);
            p += m;
            n -= m;
         }
      }

      log_printf(
         "%s%s%s%s%s",
         msg,
         details ? "; " : "",
         details,
         *stackString ? "\nStack:" : "",
         stackString
      );

      if (on_crash)
         on_crash(on_crash_ctx);

      if (IsDebuggerPresent())
         DebugBreak();

      exit(0);
   }

   return EXCEPTION_CONTINUE_SEARCH;
}

void
register_backtrace_logger(
   void (*on_crash_)(void*),
   void *ctx
)
{
   on_crash = on_crash_;
   on_crash_ctx = ctx;

   AddVectoredExceptionHandler(0, SehCallback);
}

#else

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static void
sig(int signo)
{
   void *stack[BT_DEPTH], **ps = stack;
   size_t nStack = backtrace(stack, ARRAY_SIZE(stack));
   char buf[4096];
   char *p = buf;
   int n = sizeof(buf);
   int r = 0;

   r = snprintf(p, n, "%s\nStack:", strsignal(signo));
   if (r > n)
      goto exit;
   p += r;
   n -= r;

   while (n && nStack--)
   {
      if (n >= 1)
      {
         *p++ = '\n';
         --n;
         *p = 0;
      }
      describe_symbol(*ps++, p, n);
      int m = strlen(p);
      p += m;
      n -= m;
   }

   log_printf("%s", buf); 

   if (on_crash)
      on_crash(on_crash_ctx);

#ifndef NDEBUG
   signal(SIGABRT, SIG_DFL);
   abort();
#endif

exit:
   _exit(-1);
}

void
register_backtrace_logger(
   void (*on_crash_)(void*),
   void *ctx
)
{
   struct sigaction sa;
   struct sigaction old;
   int signals[] = { SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT };

   on_crash = on_crash_;
   on_crash_ctx = ctx;

   memset(&sa, 0, sizeof(sa));

   sa.sa_handler = sig;

   for (int i = 0; i<ARRAY_SIZE(signals); ++i)
   {
      sigaction(signals[i], &sa, &old);
   }
}

#endif
