/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef misc_h
#define misc_h

#include "error.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WINDOWS)
#define strtoull _strtoui64
#define strtoll  _strtoi64
#endif

#if !defined(PID_T_FMT)
#define PID_T_FMT "d"
#endif

// These were refactored away from misc.h
#include "inline.h"
#include "minmax.h"
#include "arraysize.h"
#include "fieldoffset.h"
#include "checkatoi.h"
#include "glibccheck.h"
#include "closefrom.h"
#include "winutf.h"
#include "stdiomisc.h"

#endif
