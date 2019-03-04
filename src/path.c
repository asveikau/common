/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/path.h>
#include <common/misc.h>

#include <stdlib.h>
#include <string.h>

bool
is_dotdot(const char *name)
{
   return name &&
          *name == '.' &&
          (!name[1] || (name[1] == '.' && !name[2]));
}

#if defined(_WINDOWS)
#include <windows.h> 
#include <intsafe.h>

struct dir_enum_
{
   HANDLE FindHandle;
   WIN32_FIND_DATA FindData;
   char *DirName;
   char *LastName;
   char *FullPath;
   bool NeedFindNext;
   dir_entry Entry;
};

dir_enum
dir_enum_open(const char *path, error *err)
{
   dir_enum r = NULL;
   char *findPath = NULL;
   PWSTR widePath = NULL;

   r = malloc(sizeof(*r));
   if (!r)
      ERROR_SET(err, nomem);
   memset(r, 0, sizeof(*r));

   r->DirName = _strdup(path);
   if (!r->DirName)
      ERROR_SET(err, nomem);

   findPath = append_path(path, "*", err);
   ERROR_CHECK(err);

   widePath = ConvertToPwstr(findPath, err);
   ERROR_CHECK(err);

   r->FindHandle = FindFirstFile(widePath, &r->FindData);
   if (!r->FindHandle || r->FindHandle == INVALID_HANDLE_VALUE)
      ERROR_SET(err, win32, GetLastError());
exit:
   if (ERROR_FAILED(err))
   {
      dir_enum_close(r);
      r = NULL;
   }
   free(findPath);
   free(widePath);
   return r;
}

dir_entry *
dir_enum_read(dir_enum p, error *err)
{
   dir_entry *r = NULL;

   if (p->NeedFindNext)
   {
   retry:
      if (!FindNextFile(p->FindHandle, &p->FindData))
      {
         if (GetLastError() == ERROR_NO_MORE_FILES)
            goto exit;
         ERROR_SET(err, win32, GetLastError());
      }
      p->NeedFindNext = false;
   }

   if (p->FindData.cFileName[0] == L'.' &&
       (p->FindData.cFileName[1] == 0 ||
        (p->FindData.cFileName[1] == L'.' &&
         p->FindData.cFileName[2] == 0)))
   {
      goto retry;
   }

   r = &p->Entry;
   memset(r, 0, sizeof(*r));

   free(p->LastName);
   p->LastName = NULL;

   free(p->FullPath);
   p->FullPath = NULL;

   p->LastName = ConvertToPstr(p->FindData.cFileName, err);
   ERROR_CHECK(err);

   p->FullPath = append_path(p->DirName, p->LastName, err);
   ERROR_CHECK(err);

   r->name = p->LastName;
   r->path = p->FullPath;
   r->is_dir = (p->FindData.dwFileAttributes& FILE_ATTRIBUTE_DIRECTORY)
                  ? true : false;

   p->NeedFindNext = true;
exit:
   if (ERROR_FAILED(err))
      r = NULL;
   return r;
}

void
dir_enum_close(dir_enum p)
{
   if (p)
   {
      if (p->FindHandle && p->FindHandle != INVALID_HANDLE_VALUE)
         FindClose(p->FindHandle);
      free(p->DirName);
      free(p->LastName);
      free(p->FullPath);
      free(p);
   }
}

static bool
is_dot_or_space(char ch)
{
   return ch == '.' || ch == ' ';
}

static void
append_path_impl(
   const char *dir,
   const char *name,
   int namelen,
   char **result,
   error *err
)
{
   int required = 0;
   int dirlen = 0;
   bool has_whackwhack = false;
   bool needs_whackwhack = false;
   bool is_unc = false;
   PWSTR wide_dir = NULL;
   PWSTR full_path = NULL;
   char *heap_temp = NULL;
   char *resbuf = NULL;
   const char whackwhack[] = "\\\\?\\";
   char *p = NULL;

   has_whackwhack = !strncmp(dir, whackwhack, sizeof(whackwhack) - 1);
   if (!has_whackwhack)
   {
      if (namelen &&
          is_dot_or_space(name[namelen-1]))
      {
         needs_whackwhack = true;
      }
      else
      {
         int r = MultiByteToWideChar(
            CP_UTF8,
            0,
            dir,
            strlen(dir),
            NULL,
            0
         );
         if (!r || r > MAX_PATH - 12)
            needs_whackwhack = true; 
         else
         {
            int r2 = MultiByteToWideChar(CP_UTF8, 0, name, namelen, NULL, 0);
            if (!r2 || (r + r2 + 2) > MAX_PATH)
               needs_whackwhack = true; 
         }
      }
   }

   if (needs_whackwhack)
   {
      DWORD bufferLength = MAX_PATH / 2;
      DWORD r = 0;

      wide_dir = ConvertToPwstr(dir, err);
      ERROR_CHECK(err);

      do
      {
         HRESULT hr = S_OK;

         hr = DWordMult(bufferLength, 2 * sizeof(WCHAR), &bufferLength);
         if (FAILED(hr))
            ERROR_SET(err, win32, hr);

         if (full_path)
         {
            void *p = realloc(full_path, bufferLength);
            if (!p)
               ERROR_SET(err, nomem);
            full_path = p;
         }
         else
         {
            full_path = malloc(bufferLength);
            if (!full_path)
               ERROR_SET(err, nomem);
         }

         bufferLength /= sizeof(WCHAR);
      } while ((r = GetFullPathName(wide_dir, bufferLength, full_path, NULL)) &&
               (r > bufferLength - 1));

      heap_temp = ConvertToPstr(full_path, err);
      ERROR_CHECK(err);

      dir = heap_temp;

      is_unc = (dir[0] == '\\' && dir[1] == '\\');
      if (is_unc)
         needs_whackwhack = false;
   }

   dirlen = strlen(dir);
   required = dirlen + namelen + 2;
   if (needs_whackwhack)
      required += (sizeof(whackwhack) - 1);

   resbuf = malloc(required);
   if (!resbuf)
      ERROR_SET(err, nomem);
   p = resbuf;
   if (needs_whackwhack)
   {
      memcpy(p, whackwhack, sizeof(whackwhack) - 1);
      p += (sizeof(whackwhack) - 1);
   }
   memcpy(p, dir, dirlen);
   p += dirlen;
   *p++ = PATH_SEP;
   memcpy(p, name, namelen);
   p += namelen;
   *p = 0;

   *result = resbuf;
   resbuf = NULL;
exit:
   free(wide_dir);
   free(full_path);
   free(heap_temp);
   free(resbuf);
}

char *
append_path(
   const char *dir,
   const char *name,
   error *err
)
{
   char *result = NULL;
   char *scratch = NULL;
   const char *p = NULL;

   if (!dir || !*dir)
      ERROR_SET(err, unknown, "Null or empty directory");
   if (!name || !*name)
      ERROR_SET(err, unknown, "Null or empty filename");

   while ((p = strpbrk(name, PATH_SEP_PBRK)))
   {
      int n = p - name;

      if (n)
      {
         char *oldScratch = scratch;
         scratch = NULL;
         append_path_impl(
            dir,
            name,
            n,
            &scratch,
            err
         );
         free(oldScratch);
         ERROR_CHECK(err);
         dir = scratch;
      }

      name = p + 1;
   }

   append_path_impl(
      dir,
      name,
      strlen(name),
      &result,
      err
   );
   ERROR_CHECK(err);

exit:
   free(scratch);
   return result;
}

bool
path_is_dir(const char *path, error *err)
{
   bool r = false;
   PWSTR path16 = ConvertToPwstr(path, err);
   DWORD attrs;
   ERROR_CHECK(err);
   attrs = GetFileAttributes(path16);
   if (attrs == INVALID_FILE_ATTRIBUTES)
      ERROR_SET(err, win32, GetLastError());
   r = (attrs & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
exit:
   free(path16);
   return r;
}

bool
path_exists(const char *path, error *err)
{
   bool r = false;
   PWSTR path16 = ConvertToPwstr(path, err);
   DWORD attrs;
   ERROR_CHECK(err);
   attrs = GetFileAttributes(path16);
   if (attrs == INVALID_FILE_ATTRIBUTES)
   {
      DWORD error = GetLastError();

      switch (error)
      {
      case ERROR_FILE_NOT_FOUND:
      case ERROR_PATH_NOT_FOUND:
         goto exit;
      default:
         ERROR_SET(err, win32, error);
      }
   }

   r = true;
exit:
   free(path16);
   return r;   
}

uint64_t
get_file_size(const char *path, error *err)
{
   LARGE_INTEGER size = {0};
   PWSTR path16 = ConvertToPwstr(path, err);
   HANDLE file = INVALID_HANDLE_VALUE;

   ERROR_CHECK(err);

   file = CreateFile(
      path16,
      GENERIC_READ,
      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
      NULL,
      OPEN_EXISTING,
      FILE_ATTRIBUTE_NORMAL,
      NULL
   );
   if (file == INVALID_HANDLE_VALUE)
      ERROR_SET(err, win32, GetLastError());

   if (!GetFileSizeEx(file, &size))
      ERROR_SET(err, win32, GetLastError());
exit:
   free(path16);
   if (file != INVALID_HANDLE_VALUE)
      CloseHandle(file);
   return size.QuadPart;
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <dirent.h>
#include <errno.h>

#if 0 // this API is a bad idea, the DIR pointer should be enough state...
#if defined(__APPLE__)
#define HAVE_READDIR_R
#endif
#endif

struct dir_enum_
{
   DIR *d;
#ifdef HAVE_READDIR_R
   struct dirent storage;
#endif
   dir_entry wrappedStorage;
   char *dirname;
   char *concat;
};

struct dirent *
readdir_wrapper(dir_enum p, error *err)
{
   struct dirent *r = NULL;

#ifdef HAVE_READDIR_R
   int error_code = readdir_r(p->d, &p->storage, &r);
   if (error_code)
      ERROR_SET(err, errno, error_code);
#else
   errno = 0;
   r = readdir(p->d);
   if (!r && errno)
      ERROR_SET(err, errno, errno);
#endif

exit:
   return r;
}

bool
check_is_dir(dir_enum p, struct dirent *dirent, error *err)
{
#if !defined(__sun__)
   switch (dirent->d_type)
   {
   case DT_DIR:
      return true;
   case DT_UNKNOWN:
      break;
   default:
      return false;
   }
#endif

   return path_is_dir(p->concat, err);
}

dir_enum
dir_enum_open(const char *path, error *err)
{
   dir_enum r = malloc(sizeof(*r));
   if (!r)
      ERROR_SET(err, nomem);
   memset(r, 0, sizeof(*r));
   r->dirname = strdup(path);
   if (!r->dirname)
      ERROR_SET(err, nomem);
   r->d = opendir(path);
   if (!r->d)
      ERROR_SET(err, errno, errno);
exit:
   if (ERROR_FAILED(err))
   {
      dir_enum_close(r);
      r = NULL;
   }
   return r;
}

dir_entry *
dir_enum_read(dir_enum p, error *err)
{
   dir_entry *ent = NULL;
   struct dirent *dirent;
   bool is_dir;

   do
   {
      dirent = readdir_wrapper(p, err);
      ERROR_CHECK(err);
      if (!dirent)
         goto exit;
   } while (is_dotdot(dirent->d_name));

   free(p->concat);
   p->concat = NULL;

   p->concat = append_path(p->dirname, dirent->d_name, err);
   ERROR_CHECK(err);

   is_dir = check_is_dir(p, dirent, err);
   ERROR_CHECK(err);

   ent = &p->wrappedStorage;
   memset(ent, 0, sizeof(*ent));

   ent->name = dirent->d_name;
   ent->path = p->concat;
   ent->is_dir = is_dir;

exit:
   return ent;
}

void
dir_enum_close(dir_enum p)
{
   if (p)
   {
      free(p->dirname);
      free(p->concat);
      if (p->d)
         closedir(p->d);
      free(p);
   }
}

char *
append_path(
   const char *dir,
   const char *name,
   error *err
)
{
   char *result = NULL;
   char *p;
   int dirlen, namelen;

   if (!dir || !*dir)
      ERROR_SET(err, unknown, "Null or empty directory");
   if (!name || !*name)
      ERROR_SET(err, unknown, "Null or empty filename");

   dirlen = strlen(dir);
   namelen = strlen(name);

   result = malloc(dirlen + namelen + 2);
   if (!result)
      ERROR_SET(err, nomem);

   p = result;
   memcpy(p, dir, dirlen);
   p += dirlen;
   *p++ = PATH_SEP;
   memcpy(p, name, namelen);
   p += namelen;
   *p = 0;

exit:
   return result;
}

bool
path_is_dir(const char *path, error *err)
{
   struct stat buf;

   if (stat(path, &buf))
      ERROR_SET(err, errno, errno);

   return S_ISDIR(buf.st_mode) ? true : false;
exit:
   return false;
}

bool
path_exists(const char *path, error *err)
{
   bool r = false;
   struct stat buf;

   if (stat(path, &buf))
   {
      switch (errno)
      {
      case ENOENT:
         goto exit;
      default:
         ERROR_SET(err, errno, errno);
      }
   }

   r = true;
exit:
   return r;
}

uint64_t
get_file_size(const char *path, error *err)
{
   uint64_t size = 0;
   struct stat buf;

   if (stat(path, &buf))
      ERROR_SET(err, errno, errno);

   size = buf.st_size;
exit:
   return size;
}

#endif

bool
path_is_relative(const char *path)
{
   if (!path || !*path)
      return false;
   if (strchr(PATH_SEP_PBRK, *path))
      return false;
#if defined(_WINDOWS)
   if (((*path >= 'A' && *path <= 'Z') ||
        (*path >= 'a' && *path <= 'z')) &&
       path[1] == ':' &&
       strchr(PATH_SEP_PBRK, path[2]))
      return false;
#endif
   return true;
}
