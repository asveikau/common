/*
 Copyright (C) 2019 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/path.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined (_WINDOWS)

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || \
    defined(__NetBSD__)

#define HAVE_GET_MOUNT_POINT
#define HAVE_GET_FS_TYPE

#include <sys/param.h>
#include <sys/mount.h>

#if defined(__NetBSD__)
#include <sys/statvfs.h>
#define statfs statvfs
#endif

char *
get_mount_point(const char *path, error *err)
{
   struct statfs buf;
   char *p = NULL;

   if (statfs(path, &buf))
      ERROR_SET(err, errno, errno);

   if (!(p = strdup(buf.f_mntonname)))
      ERROR_SET(err, nomem);

exit:
   return p;
}

char *
get_fs_type(const char *path, error *err)
{
   struct statfs buf;
   char *p = NULL;

   if (statfs(path, &buf))
      ERROR_SET(err, errno, errno);

   if (!(p = strdup(buf.f_fstypename)))
      ERROR_SET(err, nomem);

exit:
   return p;
}

#elif defined(__sun__)

#define HAVE_GET_FS_TYPE

#include <sys/stat.h>

char *
get_fs_type(const char *path, error *err)
{
   struct stat buf;
   char *p = NULL;

   if (stat(path, &buf))
      ERROR_SET(err, errno, errno);

   if (!(p = strdup(buf.st_fstype)))
      ERROR_SET(err, nomem);

exit:
   return p;
}

#elif defined(__linux__)

#define HAVE_GET_FS_TYPE

#include <sys/vfs.h>
#include <linux/magic.h>

#define CIFS_MAGIC_NUMBER    0xFF534D42
#define EXT_SUPER_MAGIC      0x137d
#define EXT2_OLD_SUPER_MAGIC 0xef51
#define UFS_MAGIC            0x00011954
#define XFS_SUPER_MAGIC      0x58465342
#define JFS_SUPER_MAGIC      0x3153464a
#define NTFS_SB_MAGIC        0x5346544e

char *
get_fs_type(const char *path, error *err)
{
   struct statfs buf;
   char cpBuf[64];
   const char *cp = NULL;
   char *p = NULL;

   if (statfs(path, &buf))
      ERROR_SET(err, errno, errno);

   switch (buf.f_type)
   {
#define FSTYPE(MAGIC, STRING) case MAGIC: cp = STRING; break

   // Common remote types:
   //
   FSTYPE(NFS_SUPER_MAGIC, "nfs");
   FSTYPE(SMB_SUPER_MAGIC, "smbfs");
   FSTYPE(CIFS_MAGIC_NUMBER, "cifs");

   // Common local types:
   //
   FSTYPE(EXT_SUPER_MAGIC, "extfs");
   FSTYPE(EXT2_OLD_SUPER_MAGIC, "extfs");
   FSTYPE(EXT2_SUPER_MAGIC, "extfs");
   FSTYPE(BTRFS_SUPER_MAGIC, "btrfs");
   FSTYPE(CRAMFS_MAGIC, "cramfs");
   FSTYPE(XFS_SUPER_MAGIC, "xfs");
   FSTYPE(JFS_SUPER_MAGIC, "jfs");
   FSTYPE(JFFS2_SUPER_MAGIC, "jffs2");
   FSTYPE(REISERFS_SUPER_MAGIC, "reiserfs");

   FSTYPE(PROC_SUPER_MAGIC, "procfs");
   FSTYPE(DEVPTS_SUPER_MAGIC, "devpts");
   FSTYPE(TMPFS_MAGIC, "tmpfs");
   FSTYPE(RAMFS_MAGIC, "ramfs");
   FSTYPE(DEBUGFS_MAGIC, "debugfs");
   FSTYPE(EFIVARFS_MAGIC, "efivarsfs");

   FSTYPE(ISOFS_SUPER_MAGIC, "iso9660");
   FSTYPE(UFS_MAGIC, "ufs");
   FSTYPE(MSDOS_SUPER_MAGIC, "msdos");
   FSTYPE(NTFS_SB_MAGIC, "ntfs");

#undef FSTYPE

   default:
      snprintf(cpBuf, sizeof(cpBuf), "unknown[%lx]", buf.f_type);
      cp = cpBuf;
   }

   if (!(p = strdup(cp)))
      ERROR_SET(err, nomem);

exit:
   return p;
}

#endif

#if !defined(HAVE_GET_MOUNT_POINT) || !defined(HAVE_GET_FS_TYPE)

#include <unistd.h>
#include <spawn.h>
#include <common/buffer.h>
#include <stdarg.h>
#include <ctype.h>

static void
spawn_to_pipe(error *err, int *fd, const char *argv0, ...)
{
   int fds[2] = {-1, -1};
   buffer argv = {0};
   const char *arg = NULL;
   va_list ap;
   int r = 0;
   pid_t pid = -1;
   int flags = 0;

   posix_spawn_file_actions_t fa;
   posix_spawnattr_t sa;
   bool fa_init = false;
   bool sa_init = false;

   va_start(ap, argv0);

   if ((r = posix_spawnattr_init(&sa)))
      ERROR_SET(err, errno, r);

   sa_init = true;

   if ((r = posix_spawn_file_actions_init(&fa)))
      ERROR_SET(err, errno, r);

   fa_init = true;

   if (pipe(fds))
      ERROR_SET(err, errno, errno);

   if ((r = posix_spawn_file_actions_adddup2(&fa, fds[1], 1)))
      ERROR_SET(err, errno, r);

   if(!buffer_append(&argv, &argv0, sizeof(argv0)))
      ERROR_SET(err, nomem);

   do
   {
      arg = va_arg(ap, const char *);
      if (!buffer_append(&argv, &arg, sizeof(arg)))
         ERROR_SET(err, nomem);
   } while (arg);

   if (flags && (r = posix_spawnattr_setflags(&sa, flags)))
      ERROR_SET(err, errno, r);

   r = posix_spawnp(
      &pid,
      argv0,
      &fa,
      &sa,
      BUFFER_PTR(&argv),
      NULL
   );
   if (r)
      ERROR_SET(err, errno, r);

   *fd = fds[0];
   fds[0] = -1;

exit:
   if (fds[0] >= 0)
      close(fds[0]);
   if (fds[1] >= 0)
      close(fds[1]);
   buffer_destroy(&argv);
   if (fa_init)
      posix_spawn_file_actions_destroy(&fa);
   if (sa_init)
      posix_spawnattr_destroy(&sa);
   va_end(ap);
}

#endif

#ifndef HAVE_GET_MOUNT_POINT

char *
get_mount_point(const char *path, error *err)
{
   int fd = -1;
   char buf[4096];
   int r = 0;
   enum
   {
      SkipNl,
      ParseLine,
      WhiteSpace,
      MountPoint,
   } state = SkipNl;
   int token = 0;
   buffer mountPt = {0};
#if defined(__sun__)
   const int desiredToken = 0;
   state = MountPoint;
#else
   const int desiredToken = 5;
#endif

   // Very hacky, but running "df %s" seems to reliably give us mount points
   // across several operating systems.
   //
   spawn_to_pipe(err, &fd, "df", path, NULL);
   ERROR_CHECK(err);

   while ((r = read(fd, buf, sizeof(buf))) > 0)
   {
      int i = 0;
   retry:
      switch (state)
      {
      case SkipNl:
         for (; i<r; ++i)
         {
            if (buf[i] == '\n')
            {
               state = ParseLine;
               i++;
               goto retry;
            }
         }
         break;
      case ParseLine:
         for (; i<r; ++i)
         {
            if (isspace(buf[i]))
            {
               token++;
               i++;
               state = WhiteSpace;
               goto retry;
            }
         }
         break;
      case WhiteSpace:
         for (; i<r; ++i)
         {
            if (!isspace(buf[i]))
            {
               state = (token == desiredToken) ? MountPoint : ParseLine;
               goto retry;
            }
         }
         break;
      case MountPoint:
         for (; i<r; ++i)
         {
            if (buf[i] == '\n')
               goto done;

            if (!buffer_append(&mountPt, buf+i, 1))
               ERROR_SET(err, nomem);
         }
         break;
      }
   }

done:

   if (!BUFFER_NBYTES(&mountPt))
      ERROR_SET(err, unknown, "Could not find mount point");

   buf[0] = 0;
   if (!buffer_append(&mountPt, buf, 1))
      ERROR_SET(err, nomem);

#if defined(__sun__)
   {
      char *p = strstr(BUFFER_PTR(&mountPt), "  (");
      if (p)
         *p = 0;
   }
#endif

exit:
   if (fd >= 0)
      close(fd);
   if (ERROR_FAILED(err))
   {
      buffer_destroy(&mountPt);
      return NULL;
   }
   return BUFFER_PTR(&mountPt);
}

#endif

#ifndef HAVE_GET_FS_TYPE

char *
get_fs_type(const char *path, error *err)
{
   // TODO
   error_set_errno(err, ENOSYS);
}

#endif

bool
path_is_remote(const char *path, error *err)
{
   bool r = false;
   // TODO: consider parsing /etc/dfs/fstypes on Solaris
   static const char *remote_filesystems[] =
   {
      "nfs",
      "smb",
      "smbfs",
      "cifs",
      NULL
   };
   const char **remote_fs = remote_filesystems;
   char *fs = get_fs_type(path, err);

   ERROR_CHECK(err);

   for (; *remote_fs; ++remote_fs)
   {
      if (!strcmp(*remote_fs, fs))
      {
         r = true;
         break;
      }
   }

exit:
   free(fs);
   return r;
}

#else

bool
path_is_remote(const char *path, error *err)
{
   return false;
}

#endif
