/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#if defined(__linux__)
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <common/misc.h>

int
closefrom(int minfd)
{
   char buf[128];
   DIR *dir = NULL;
   struct dirent *ent;
   int r = 0;
   int fd;

   snprintf(buf, sizeof(buf), "/proc/%" PID_T_FMT "/fd", getpid());

   dir = opendir(buf);

   if (!dir)
      r = -1;
   else
   {
      while ((ent = readdir(dir)))
      {
         if (ent->d_name[0] == '.' &&
             (!ent->d_name[1] || (ent->d_name[1] == '.' && !ent->d_name[2])))
            continue;

         fd = atoi(ent->d_name);
         if (fd >= minfd && fd != dirfd(dir))
            close(fd);
      }

      closedir(dir);
   }

   return r;
}
#endif

#if defined (__APPLE__)
#include <unistd.h>

int
closefrom(int minfd)
{
   int r = getdtablesize();
   for (; minfd <= r; ++minfd)
      close(minfd);
   return 0;
}
#endif
