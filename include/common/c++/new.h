/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_cxx_new_h_
#define common_cxx_new_h_

#include <new>
#include <common/error.h>

namespace common {

template <typename T>
void
New(T **ptr, error *err)
{
   try
   {
      *ptr = new T();
   }
   catch (std::bad_alloc)
   {
      *ptr = nullptr;
      ERROR_SET(err, nomem);
   }
exit:;
}

} // end namespace

#endif