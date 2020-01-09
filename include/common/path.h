/*
 Copyright (C) 2017, 2018, 2019 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_path_h
#define common_path_h

#include "error.h"
#include <stdbool.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef PATH_SEP
#if defined(_WINDOWS)
#define PATH_SEP      '\\'
#define PATH_SEP_PBRK "\\/"
#else
#define PATH_SEP      '/'
#define PATH_SEP_PBRK "/"
#endif
#endif

struct dir_enum_;
typedef struct dir_enum_ *dir_enum;
typedef struct
{
   const char *name; // just the name part
   const char *path; // concatenation of parent dir name and name
   bool is_dir;      // true if this is a directory
} dir_entry;

dir_enum
dir_enum_open(const char *path, error *err);

dir_entry *
dir_enum_read(dir_enum p, error *err);

void
dir_enum_close(dir_enum p);

bool
is_dotdot(const char *str);

char *
append_path(
   const char *dir,
   const char *file,
   error *err
);

bool
path_is_dir(const char *dir, error *err);

bool
path_exists(const char *path, error *err);

bool
path_is_relative(const char *path);

// Returns NULL if path is already absolute.
//
char *
make_absolute_path(const char *path, error *err);

char *
sanitize_path(const char *path, error *err);

bool
path_is_remote(const char *path, error *err);

#if defined(_WINDOWS)
bool
fd_is_remote(HANDLE fd, error *err);
#else
bool
fd_is_remote(int fd, error *err);
#endif

uint64_t
get_file_size(const char *path, error *err);

char *
get_cwd(error *err);

enum private_dir_flags
{
   PRIVATE_DIR_ROAMING = (1<<0),
   PRIVATE_DIR_CACHE   = (1<<1),
};

char *
get_private_dir(int flags, error *err);

char *
get_private_subdir(
   const char *subdir,
   int flags,
   error *err
);

void
secure_mkdir(const char *path, error *err);

void
mkdir_if_not_exists(const char *path, error *err);

void
copy_file(const char *src, const char *dst, error *err);

const char *
get_appname();

void
set_appname(const char *str);

const char *
get_program_path(error *err);

void
libcommon_set_argv0(const char *progname);

const char *
get_bundle_path();

void
libcommon_set_bundle_path(const char *path, error *err);

#if defined(__cplusplus)
}
#endif
#endif
