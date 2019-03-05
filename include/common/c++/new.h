/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_cxx_new_h_
#define common_cxx_new_h_

#include <new>
#include <memory>
#include <common/error.h>
#include <common/c++/refcount.h>

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

template <typename T>
void
New(common::Pointer<T> &ptr, error *err)
{
   New(ptr.GetAddressOf(), err);
}

template <typename T>
void
New(std::shared_ptr<T> &ptr, error *err)
{
   T *p = nullptr;
   try
   {
      p = new T();
      ptr = std::shared_ptr<T>(p);
      p = nullptr;
   }
   catch (std::bad_alloc)
   {
      ERROR_SET(err, nomem);
   }
exit:
   if (p)
      delete p;
}

template <typename T>
void
New(std::unique_ptr<T> &ptr, error *err)
{
   T *p = nullptr;
   try
   {
      p = new T();
      ptr = std::unique_ptr<T>(p);
      p = nullptr;
   }
   catch (std::bad_alloc)
   {
      ERROR_SET(err, nomem);
   }
exit:
   if (p)
      delete p;
}

} // end namespace

#endif