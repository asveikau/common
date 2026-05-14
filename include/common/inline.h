/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef libcommon_inline_h
#define libcommon_inline_h

#ifndef INLINE
#if defined(__cplusplus)
#define INLINE inline
#elif defined(_MSC_VER)
#define INLINE __declspec(inline)
#elif defined(__GNUC__)
#define INLINE __inline__
#else
#error
#endif
#endif

#endif
