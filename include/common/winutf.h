/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef libcommon_winutf_h
#define libcommon_winutf_h
#if defined(_WINDOWS)

#include <windows.h>
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

PWSTR
ConvertToPwstr(PCSTR utf8, error *err);

PSTR
ConvertToPstr(PCWSTR utf16, error *err);

#ifdef __cplusplus
}
#endif  // C++
#endif  // Windows
#endif  // header
