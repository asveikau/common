/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef libcommon_stdiomisc_h
#define libcommon_stdiomisc_h

#include <stdio.h>

#if defined(_WINDOWS)
#include <io.h>
#include <fcntl.h>

#define stdio_set_binary(fp) _setmode(_fileno(fp), _O_BINARY)

#define ftello _ftelli64
#define fseeko _fseeki64

#if defined(__cplusplus)
extern "C" {
#endif

int
asprintf(char **dst, const char *fmt, ...);

int
vasprintf(char **dst, const char *fmt, va_list ap);

#if defined(__cplusplus)
}
#endif

#else

//
// Non-Windows ...
//

#define stdio_set_binary(fp) ((void)0)
#endif

#endif
