/*
 Copyright (C) 2017, 2018, 2019 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/logger.h>
#include <common/path.h>
#include <common/misc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WINDOWS)

struct log_context
{
   HANDLE LogFile;
   int id;
};

#define LOG_CONTEXT_INIT {INVALID_HANDLE_VALUE, -1}

static void
log_init(struct log_context *ctx, const char *path, error *err)
{
   PWSTR path16 = NULL;

   path16 = ConvertToPwstr(path, err);
   ERROR_CHECK(err);

   ctx->LogFile = CreateFile(
      path16,
      GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      NULL,
      OPEN_ALWAYS,
      FILE_ATTRIBUTE_NORMAL,
      NULL
   );

exit:
   free(path16);
}

static void
log_destroy(struct log_context *ctx)
{
   if (ctx->LogFile != INVALID_HANDLE_VALUE)
   {
      CloseHandle(ctx->LogFile);
      ctx->LogFile = INVALID_HANDLE_VALUE;
   }
   ctx->id = -1;
}

static void
log_cb_platform(struct log_context *context, const char *msg)
{
   int msglen = strlen(msg);

   if (context->LogFile && context->LogFile != INVALID_HANDLE_VALUE)
   {
      OVERLAPPED ol = {0};
      DWORD bytesWritten = 0;
      ol.Offset = ~0U;
      ol.OffsetHigh = ~0U;
      WriteFile(context->LogFile, msg, msglen, &bytesWritten, &ol);
   }

   if (IsDebuggerPresent())
   {
      int n = msglen;
      PWSTR utf16 = _alloca((n + 1) * sizeof(WCHAR));
      int r = MultiByteToWideChar(
         CP_UTF8,
         0,
         msg,
         n,
         utf16,
         n
      );
      if (r)
      {
         utf16[r] = 0;
         OutputDebugString(utf16);
      }
   }
}

#else

#include <common/thread.h>
#include <unistd.h>
#include <fcntl.h>

struct log_context
{
   FILE *logfile;
   FILE *stderr_clone;
   int stderr_pipe[2];
   thread_id stderr_thread;
   int id;
};

#define LOG_CONTEXT_INIT { NULL, NULL, {-1, -1}, {NULL}, -1 }

static void
on_log_from_stderr(const char *msg)
{
   if (*msg)
      log_printf("stderr: %s", msg);
}

static
THREAD_PROC_RETVAL
stderr_thread_proc(void *arg)
{
   struct log_context *ctx = arg;
   int fd = ctx->stderr_pipe[0];
   error err;
   char buf[4096];
   int offset = 0;

   ctx->stderr_pipe[0] = -1;
   memset(&err, 0, sizeof(err));

   for (;;)
   {
      int r = read(fd, buf + offset, sizeof(buf) - offset - 1);
      if (r == 0)
         break;
      else if (r < 0 && (errno != EINTR && errno != EAGAIN))
         ERROR_SET(&err, errno, errno);
      else if (r > 0)
      {
         const char *p = buf;

         r += offset;

         for (offset = 0; offset < r; ++offset)
         {
            if (buf[offset] == '\n')
            {
               buf[offset] = 0;
               on_log_from_stderr(p);
               p = buf + offset + 1;
            }
         }

         if (p == buf)
         {
            buf[sizeof(buf) - 1] = 0;
            on_log_from_stderr(p);
         }
         else if (p != buf + r)
         {
            offset = r - (p-buf);
            memmove(buf, p, offset);
         }
         else
         {
            offset = 0;
         }
      }
   }

exit:
   close(fd);
   error_clear(&err);
   return 0;
}

static void
log_init(struct log_context *ctx, const char *path, error *err)
{
   int fd;

   fd = open(path, O_CREAT | O_APPEND | O_WRONLY, 0600);
   if (fd >= 0)
   {
      ctx->logfile = fdopen(fd, "a");
      if (!ctx->logfile)
         close(fd);
      else
         setvbuf(ctx->logfile, NULL, _IONBF, 0);
   }

   fd = dup(2);
   if (fd >= 0)
   {
      ctx->stderr_clone = fdopen(fd, "a");
      if (!ctx->stderr_clone)
         close(fd);
      else
         setvbuf(ctx->stderr_clone, NULL, _IONBF, 0);
   }

   if (pipe(ctx->stderr_pipe))
      ERROR_SET(err, errno, errno);

   create_thread(ctx, stderr_thread_proc, &ctx->stderr_thread, err);
   ERROR_CHECK(err);

   if (dup2(ctx->stderr_pipe[1], 2) != 2)
      ERROR_SET(err, errno, errno);

exit:
   if (!thread_is_started(&ctx->stderr_thread) && ctx->stderr_pipe[0] >= 0)
   {
      close(ctx->stderr_pipe[0]);
      ctx->stderr_pipe[0] = -1;
   }
   if (ctx->stderr_pipe[1] >= 0)
   {
      close(ctx->stderr_pipe[1]);
      ctx->stderr_pipe[1] = -1;
   }
}

static void
close_fd(int *pfd)
{
   if (pfd && *pfd >= 0)
   {
      close(*pfd);
      *pfd = -1;
   }
}

static void
log_destroy(struct log_context *ctx)
{
   // Need to restore old stderr fd first, so that any new writes
   // to stderr don't go to the broken pipe.
   //
   if (ctx->stderr_clone)
      dup2(fileno(ctx->stderr_clone), 2);

   close_fd(&ctx->stderr_pipe[1]);

   if (!thread_is_started(&ctx->stderr_thread))
      close_fd(&ctx->stderr_pipe[0]);
   else
   {
      join_thread(&ctx->stderr_thread);
      memset(&ctx->stderr_thread, 0, sizeof(ctx->stderr_thread));
   }

   if (ctx->stderr_clone)
   {
      fflush(ctx->stderr_clone);
      fclose(ctx->stderr_clone);
      ctx->stderr_clone = NULL;
   }

   if (ctx->logfile)
   {
      fclose(ctx->logfile);
      ctx->logfile = NULL;
   }

   ctx->id = -1;
}

static void
log_cb_platform(struct log_context *context, const char *msg)
{
   FILE *files[] = { context->logfile, context->stderr_clone };
   int i;

   for (i=0; i<ARRAY_SIZE(files); ++i)
   {
      FILE *fp = files[i];
      if (fp)
         fputs(msg, fp);
   }
}

#endif

static void
log_cb(void *contextp, const char *msg)
{
   struct log_context *context = contextp;

   log_cb_platform(context, msg);
}

static struct log_context ctx = LOG_CONTEXT_INIT;

void
log_register_default_callback()
{
   char *dir = NULL;
   char *logfile = NULL;

   error err;
   memset(&err, 0, sizeof(err));

   dir = get_private_dir(0, &err);
   ERROR_CHECK(&err);

   logfile = append_path(dir, "log", &err);
   ERROR_CHECK(&err);

   log_init(&ctx, logfile, &err);
   ERROR_CHECK(&err);

   ctx.id = log_register_callback(log_cb, &ctx);
exit:
   free(dir);
   free(logfile);
   error_clear(&err);
}

bool
log_unregister_default_callback()
{
   bool r = false;
   if (ctx.id >= 0)
   {
      log_unregister_callback(ctx.id);
      log_destroy(&ctx);
      r = true;
   }
   return r;
}
