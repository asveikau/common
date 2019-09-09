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
#define HAVE_STATFS

#include <sys/param.h>
#include <sys/mount.h>

#if defined(__NetBSD__)
#include <sys/statvfs.h>
#define statfs  statvfs
#define fstatfs fstatvfs
#endif


static
char *
get_mount_point_statfs(const struct statfs *buf, error *err)
{
   char *p = NULL;

   if (!(p = strdup(buf->f_mntonname)))
      ERROR_SET(err, nomem);

exit:
   return p;
}

char *
get_mount_point(const char *path, error *err)
{
   struct statfs buf;
   char *p = NULL;

   if (statfs(path, &buf))
      ERROR_SET(err, errno, errno);

   p = get_mount_point_statfs(&buf, err);
   ERROR_CHECK(err);

exit:
   return p;
}

char *
get_mount_point_fd(int fd, error *err)
{
   struct statfs buf;
   char *p = NULL;

   if (fstatfs(fd, &buf))
      ERROR_SET(err, errno, errno);

   p = get_mount_point_statfs(&buf, err);
   ERROR_CHECK(err);

exit:
   return p;
}

static
char *
get_fs_type_statfs(const struct statfs *buf, error *err)
{
   char *p = NULL;

   if (!(p = strdup(buf->f_fstypename)))
      ERROR_SET(err, nomem);

exit:
   return p;
}

#elif defined(__sun__)

#define HAVE_GET_FS_TYPE

#include <sys/stat.h>

static
char *
get_fs_type_stat(const struct stat *buf, error *err)
{
   char *p = NULL;
   if (!(p = strdup(buf->st_fstype)))
      ERROR_SET(err, nomem);
exit:
   return p;
}

char *
get_fs_type(const char *path, error *err)
{
   struct stat buf;
   char *p = NULL;

   if (stat(path, &buf))
      ERROR_SET(err, errno, errno);

   p = get_fs_type_stat(&buf, err);
   ERROR_CHECK(err);

exit:
   return p;
}

char *
get_fs_type_fd(int fd, error *err)
{
   struct stat buf;
   char *p = NULL;

   if (fstat(fd, &buf))
      ERROR_SET(err, errno, errno);

   p = get_fs_type_stat(&buf, err);
   ERROR_CHECK(err);

exit:
   return p;
}

#elif defined(__linux__)

#define HAVE_STATFS

#include <sys/vfs.h>
#include <linux/magic.h>

#define CIFS_MAGIC_NUMBER    0xFF534D42
#define EXT_SUPER_MAGIC      0x137d
#define EXT2_OLD_SUPER_MAGIC 0xef51
#define UFS_MAGIC            0x00011954
#define XFS_SUPER_MAGIC      0x58465342
#define JFS_SUPER_MAGIC      0x3153464a
#define NTFS_SB_MAGIC        0x5346544e

static char *
get_fs_type_statfs(const struct statfs *buf, error *err)
{
   char cpBuf[64];
   const char *cp = NULL;
   char *p = NULL;

   switch (buf->f_type)
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
      snprintf(cpBuf, sizeof(cpBuf), "unknown[%lx]", buf->f_type);
      cp = cpBuf;
   }

   if (!(p = strdup(cp)))
      ERROR_SET(err, nomem);

exit:
   return p;
}

#endif

#if defined(HAVE_STATFS)

#define HAVE_GET_FS_TYPE

char *
get_fs_type(const char *path, error *err)
{
   struct statfs buf;
   char *p = NULL;

   if (statfs(path, &buf))
      ERROR_SET(err, errno, errno);

   p = get_fs_type_statfs(&buf, err);
   ERROR_CHECK(err);

exit:
   return p;
}

char *
get_fs_type_fd(int fd, error *err)
{
   struct statfs buf;
   char *p = NULL;

   if (fstatfs(fd, &buf))
      ERROR_SET(err, errno, errno);

   p = get_fs_type_statfs(&buf, err);
   ERROR_CHECK(err);

exit:
   return p;
}

#endif

#ifndef HAVE_GET_FS_TYPE

char *
get_fs_type(const char *path, error *err)
{
   // TODO
   error_set_errno(err, ENOSYS);
}

char *
get_fs_type_fd(int fd, error *err)
{
   // TODO
   error_set_errno(err, ENOSYS);
}

#endif

static
bool
fs_type_is_remote(const char *fstype, error *err)
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

   for (; *remote_fs; ++remote_fs)
   {
      if (!strcmp(*remote_fs, fstype))
      {
         r = true;
         break;
      }
   }

   return r;
}

bool
path_is_remote(const char *path, error *err)
{
   char *fs = NULL;
   bool r = false;

   fs = get_fs_type(path, err);
   ERROR_CHECK(err);

   r = fs_type_is_remote(fs, err);
   ERROR_CHECK(err);

exit:
   free(fs);
   return r;
}

bool
fd_is_remote(int fd, error *err)
{
   char *fs = NULL;
   bool r = false;

   fs = get_fs_type_fd(fd, err);
   ERROR_CHECK(err);

   r = fs_type_is_remote(fs, err);
   ERROR_CHECK(err);

exit:
   free(fs);
   return r;
}

#else // Windows

bool
path_is_remote(const char *path, error *err)
{
   return false;
}

bool
fd_is_remote(HANDLE fd, error *err)
{
   return false;
}

#endif
