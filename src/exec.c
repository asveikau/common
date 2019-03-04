/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/exec.h>
#include <common/misc.h>

#include <unistd.h>
#include <stdlib.h>

struct pipefd
{
   enum
   {
      READ = 0,
      WRITE = 1
   } op;
   int fds[2];
   int target_fd;
   FILE **file;
};

static int
opposite_fd(int op)
{
   return 1 - op;
}

static int
init_pipe(struct pipefd *p, int targetfd, int op, FILE **file, int *count)
{
   int r = 0;
   p->op = op;
   p->target_fd = targetfd;
   p->file = file;
   if (!(r = pipe(p->fds)))
      ++*count;
   return r;
}

static void
safeclose(int fd)
{
   if (fd >= 0)
      close(fd);
}

extern char **environ;

static int
post_fork(
   struct popen_exec_args *args
)
{
   if ((args->flags) & POPEN_SEARCH_PATH)
   {
      if (args->environ)
      {
#ifdef _GNU_SOURCE
         return execvpe(*args->argv, args->argv, args->environ);
#else
         environ = (char**)args->environ;
         return execvp(*args->argv, args->argv); 
#endif
      }
      else
      {
         return execvp(*args->argv, args->argv);
      }
   }
   else
   {
      char *const *envp = args->environ ? args->environ : environ;

      return execve(*args->argv, args->argv, envp);
   }
}

int
popen_exec(
   struct popen_exec_args *args
)
{
   int r = 0;
   struct pipefd pipes[3];
   int npipes = 0;
   struct pipefd *p, *q;
   pid_t pid;
   int stderr_to_stdout = 0;

   if (args->input_pipe &&
       (r = init_pipe(&pipes[npipes], READ, 0, args->input_pipe, &npipes)))
   {
      goto exit; 
   }

   if (args->output_pipe &&
       (r = init_pipe(&pipes[npipes], WRITE, 1, args->output_pipe, &npipes)))
   {
      goto exit; 
   }

   if (args->error_pipe &&
       !(stderr_to_stdout = (args->output_pipe == args->error_pipe)) &&
       (r = init_pipe(&pipes[npipes], WRITE, 2, args->error_pipe, &npipes)))
   {
      goto exit; 
   }

   pid = fork();
   if (pid < 0)
      goto exit;
   else if (pid == 0)
   {
      for (p = pipes, q = p + npipes; p < q; ++p)
      {
         dup2(p->fds[p->op], p->target_fd);
         close(p->fds[opposite_fd(p->op)]);
      }

      if (stderr_to_stdout)
         dup2(1, 2);

      closefrom(3);

      exit(post_fork(args));
   }

   for (p = pipes, q = p + npipes; p < q; ++p)
   {
      static const char *modes[] = { "r", "w" };
      int idx = opposite_fd(p->op);

      if (!(*p->file = fdopen(p->fds[idx], modes[idx])))
      {
         r = -1;
         goto exit;
      }

      // FILE struct now owns the fd.
      //
      p->fds[idx] = -1;
   }

exit:
   while (npipes > 0)
   {
      struct pipefd *p = &pipes[--npipes];
      safeclose(p->fds[0]);
      safeclose(p->fds[1]);
      if (r && *p->file)
      {
         fclose(*p->file);
         *p->file = NULL;
      }
   }
   return r;
}
