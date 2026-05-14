/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef libcommon_arraysize_h
#define libcommon_arraysize_h

#include <stddef.h>

#if !defined(ARRAY_SIZE)
#if defined(__cplusplus)
namespace common { namespace internal
{
   template<class T, size_t N>
   char (&SizeHelper(T (&arr)[N]))[N];
} } // end namespace
#define ARRAY_SIZE(X) sizeof(common::internal::SizeHelper(X))
#else
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(*X))
#endif
#endif

#endif
