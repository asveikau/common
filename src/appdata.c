/*
 Copyright (C) 2017, 2018, 2019 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/error.h>
#include <common/misc.h>
#include <common/path.h>
#include <common/size.h>
#include <common/logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(_WINDOWS)
#include <windows.h>
#include <shlobj.h>
#include <sddl.h>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "advapi32.lib")

static char *
get_appdata(int flags, error *err)
{
   HRESULT hr = S_OK;
   PWSTR onComHeap = NULL;
   char *r = NULL;
   PCWSTR psz = NULL;
   BOOL appendTemp = FALSE;
   WCHAR sz[MAX_PATH];
#if XP_SUPPORT
   HMODULE shell32 = GetModuleHandle(L"shell32.dll");
   HRESULT (WINAPI *SHGetKnownFolderPath)
        (REFKNOWNFOLDERID, DWORD, HANDLE, PWSTR*)
      = (PVOID)GetProcAddress(shell32, "SHGetKnownFolderPath");
#endif

   if ((flags & PRIVATE_DIR_CACHE))
   {
      static const PCWSTR vars[] =
      {
         L"TMP", L"TEMP", NULL
      };
      static const PCWSTR **p = vars;
      DWORD n = 0;

      while (*p)
      {
         PCWSTR var = *p++;
         DWORD n = GetEnvironmentVariable(var, sz, ARRAY_SIZE(sz));
         if (n == 0)
         {
            DWORD error = GetLastError();
            if (error == ERROR_ENVVAR_NOT_FOUND)
               continue;
            ERROR_SET(err, win32, error);
         }
         else if (n >= ARRAY_SIZE(sz))
         {
            log_printf("warning: %ls does not fit in PATH_MAX", var);
            continue;
         }

         psz = sz;
         break;
      }

      if (psz)
         goto found;

      flags &= ~PRIVATE_DIR_ROAMING;
      appendTemp = TRUE;
   }

   if (SHGetKnownFolderPath)
   {
      hr = SHGetKnownFolderPath(
         (flags & PRIVATE_DIR_ROAMING) ? &FOLDERID_RoamingAppData : &FOLDERID_LocalAppData, 
         0,
         NULL,
         &onComHeap
      );
      if (FAILED(hr)) ERROR_SET(err, win32, hr);
      psz = onComHeap;
   }
#if XP_SUPPORT
   else
   {
      hr = SHGetFolderPath(
         NULL,
         (flags & PRIVATE_DIR_ROAMING) ? CSIDL_APPDATA : CSIDL_LOCAL_APPDATA,
         NULL,
         0,
         sz
      );
      if (FAILED(hr)) ERROR_SET(err, win32, hr);
      psz = sz;
   }
#endif

found:

   r = ConvertToPstr(psz, err);
   ERROR_CHECK(err);

   if (appendTemp)
   {
      char *dupe = append_path(r, "Temp", err);
      ERROR_CHECK(err);
      free(r);
      r = dupe;

      mkdir_if_not_exists(r, err);
      ERROR_CHECK(err);
   }

exit:
   if (onComHeap) CoTaskMemFree(onComHeap); 
   return r;
}

static PWSTR
GetUserSidString(error *err)
{
   PWSTR r = NULL;

   HANDLE tokenHandle = INVALID_HANDLE_VALUE;
   union
   {
      TOKEN_USER user;
      char buffer[4096];
   } tokenUser;
   DWORD dummy = 0;

   if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &tokenHandle))
      ERROR_SET(err, win32, GetLastError());

   if (!GetTokenInformation(tokenHandle, TokenUser, &tokenUser,
           sizeof(tokenUser), &dummy))
      ERROR_SET(err, win32, GetLastError());

   if (!ConvertSidToStringSid(tokenUser.user.User.Sid, &r))
      ERROR_SET(err, win32, GetLastError());

exit:
   if (ERROR_FAILED(err))
   {
      LocalFree(r);
      r = NULL;
   }
   if (tokenHandle != INVALID_HANDLE_VALUE)
      CloseHandle(tokenHandle);
   return r;
}

void
mkdir_if_not_exists(const char *path, error *err)
{
   PWSTR path16 = ConvertToPwstr(path, err);
   ERROR_CHECK(err);
   if (!CreateDirectory(path16, NULL))
   {
      if (GetLastError() != ERROR_ALREADY_EXISTS)
         ERROR_SET(err, win32, GetLastError());
   }
exit:
   free(path16);
}

void
secure_mkdir(const char *path, error *err)
{
   PWSTR path16 = NULL;
   static const WCHAR sddl[] =
      L"D:PAR"            /* DACL: don't inherit, children inherit this */
      L"(A;OICI;FA;;;%s)" /* allow %s */
      ;
   PSECURITY_DESCRIPTOR pSD = NULL;
   ULONG sdLen = 0;
   SECURITY_ATTRIBUTES attrs = {0};
   PWSTR userSid = NULL;
   PWSTR sddlOnHeap = NULL;
   size_t sddlLen = 0;

   path16 = ConvertToPwstr(path, err);
   ERROR_CHECK(err);

   userSid = GetUserSidString(err);
   ERROR_CHECK(err);

   if (size_add(ARRAY_SIZE(sddl), wcslen(userSid), &sddlLen) ||
       size_mult(sddlLen, sizeof(WCHAR), &sddlLen))
      ERROR_SET(err, unknown, "Integer overflow");

   sddlOnHeap = malloc(sddlLen);
   if (!sddlOnHeap)
      ERROR_SET(err, nomem);

   swprintf(sddlOnHeap, sddlLen/sizeof(WCHAR), sddl, userSid);

   if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
      sddlOnHeap,
      SDDL_REVISION_1,
      &pSD,
      &sdLen
   ))
   {
      ERROR_SET(err, win32, GetLastError());
   }

   attrs.nLength = sizeof(attrs);
   attrs.lpSecurityDescriptor = pSD;

   if (!CreateDirectory(path16, &attrs))
   {
      if (GetLastError() != ERROR_ALREADY_EXISTS)
         ERROR_SET(err, win32, GetLastError());

      // TODO: validate security of existing dir, like in
      // unix version with stat call.
   }

exit:
   free(path16);
   if (pSD)
      LocalFree(pSD);
   if (userSid)
      LocalFree(userSid);
   free(sddlOnHeap);
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>

#if defined(__APPLE__) || defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#define HAVE_GETPWUID_R 1
#endif

static char *
get_home_dir(error *err)
{
   struct passwd *pwd = NULL;
   const char *p = NULL;
   char *onHeap = NULL;

   p = getenv("HOME");
   if (p)
      goto found;

#undef ERROR_JMP
#define ERROR_JMP() ((void)0)
#if HAVE_GETPWUID_R
   char buf[16384];
   struct passwd pwd_;
   int r = getpwuid_r(getuid(), &pwd_, buf, sizeof(buf), &pwd);
   if (r)
      ERROR_SET(err, errno, r);
#else
   pwd = getpwuid(getuid());
   if (!pwd)
      ERROR_SET(err, errno, errno);
#endif
#undef ERROR_JMP
#define ERROR_JMP() goto exit
   if (ERROR_FAILED(err))
      error_clear(err);
   else if (pwd)
      p = pwd->pw_dir;

   if (!p)
      ERROR_SET(err, unknown, "Could not find home dir");

found:
   onHeap = strdup(p);
   if (!onHeap)
      ERROR_SET(err, nomem);

exit:
   return onHeap;
}

static char *
get_xdg_home_dir(const char *env, const char *def, error *err)
{
   const char *p = getenv(env);
   char *home = NULL;
   char *onHeap = NULL;

   if (!p)
   {
      home = get_home_dir(err);
      ERROR_CHECK(err);

      onHeap = append_path(home, def, err);
      ERROR_CHECK(err);
   }

   if (!onHeap)
   {
      onHeap = strdup(p);
      if (!onHeap)
         ERROR_SET(err, nomem);
   }

   mkdir_if_not_exists(onHeap, err);
   ERROR_CHECK(err);

exit:
   free(home);
   if (ERROR_FAILED(err))
   {
      free(onHeap);
      onHeap = NULL;
   }
   return onHeap;
}

static int
mkdir_helper(const char *path, struct stat *buf, error *err)
{
   int r = 0;
   if ((r = mkdir(path, 0700)))
   {
      if (errno != EEXIST)
         ERROR_SET(err, errno, errno);
      
      if (stat(path, buf))
         ERROR_SET(err, errno, errno);

      if ((buf->st_mode & S_IFMT) != S_IFDIR)
         ERROR_SET(err, unknown, "Unusual file type");
   }
exit:
   if (ERROR_FAILED(err))
      r = 0;
   return r;
}

void
secure_mkdir(const char *path, error *err)
{
   struct stat buf;

   if (mkdir_helper(path, &buf, err))
   {
      if ((buf.st_mode & 077) && chmod(path, 0700)) 
         ERROR_SET(err, errno, errno);
   }
exit:;
}

void
mkdir_if_not_exists(const char *path, error *err)
{
   struct stat buf;

   mkdir_helper(path, &buf, err);
}

#endif

char *
get_private_dir(int flags, error *err)
{
   char *r = NULL;
   char *legacy = NULL;
   char *home = NULL;
   const char *appname = get_appname();

#if defined(_WINDOWS)
   home = get_appdata(flags, err);
   ERROR_CHECK(err);
#else

   const char *env = "XDG_CONFIG_HOME";
   const char *subdir = ".config";

   if (!(flags & PRIVATE_DIR_CACHE))
   {
      //
      // First check legacy...
      //
      home = get_home_dir(err);
      ERROR_CHECK(err);
      char scratch[4096];
      snprintf(scratch, sizeof(scratch), ".%s", appname);
      legacy = append_path(home, scratch, err);
      ERROR_CHECK(err);
   }
   else
   {
      env = "XDG_CACHE_HOME";
      subdir = ".cache";
   }

   free(home);
   home = get_xdg_home_dir(env, subdir, err);
   ERROR_CHECK(err);

   if (legacy && path_exists(legacy, err))
   {
      //
      // Attempt to move to XDG loation.
      //

      r = append_path(home, appname, err);
      ERROR_CHECK(err);

      if (!path_exists(r, err))
      {
         ERROR_CHECK(err);

         if (rename(legacy, r))
            ERROR_SET(err, errno, errno);
      }
   }

#endif

   if (!r)
   {
      r = append_path(home, appname, err);
      ERROR_CHECK(err);
   }

   secure_mkdir(r, err);
   ERROR_CHECK(err);

#if !defined(_WINDOWS)
   if (!(flags & PRIVATE_DIR_ROAMING) &&
       path_is_remote(r, err))
   {
      char hostname[4+256] = "nfs_";
      char *p = NULL;

      if (gethostname(hostname+4, sizeof(hostname)-4))
         ERROR_SET(err, errno, errno);

      p = strchr(hostname, '.');
      if (p)
         *p = 0;

      p = append_path(r, hostname, err);
      ERROR_CHECK(err);
      free(r);
      r = p;

      secure_mkdir(r, err);
      ERROR_CHECK(err);
   }
   ERROR_CHECK(err);
#endif

exit:
   free(home);
   free(legacy);
   if (ERROR_FAILED(err))
   {
      free(r);
      r = NULL;
   }
   return r;
}

char *
get_private_subdir(
   const char *subdir,
   int flags,
   error *err
)
{
   char *r = NULL;
   char *parent = NULL;
   const char *p = NULL;

   parent = get_private_dir(flags, err);
   ERROR_CHECK(err);

   while ((p = strpbrk(subdir, PATH_SEP_PBRK)))
   {
      int n = p - subdir;
      if (n == 0)
      {
         subdir = p+1;
         continue;
      }

#if defined(_WINDOWS)
      char *buf = malloc(n+1);
      if (!buf) ERROR_SET(err, nomem);
#else
      char buf[n + 1];
#endif

      memcpy(buf, subdir, n);
      buf[n] = 0;

      char *scratch = append_path(parent, buf, err);
#if defined(_WINDOWS)
      free(buf);
#endif
      ERROR_CHECK(err);

      free(parent);
      parent = scratch;
      secure_mkdir(parent, err);
      ERROR_CHECK(err);

      subdir = p+1;
   }

   r = append_path(parent, subdir, err);
   ERROR_CHECK(err);

   secure_mkdir(r, err);
   ERROR_CHECK(err);
exit:
   free(parent);
   return r;
}

static const char *
default_appname = NULL;

const char *
get_appname()
{
   return default_appname;
}

void 
set_appname(const char *str)
{
   default_appname = str;
}
