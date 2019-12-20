/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_cpp_refcount_h
#define common_cpp_refcount_h

#include "../refcnt.h"

namespace common {

class RefCountable
{
   refcnt ref;
public:
   RefCountable() : ref(1) {}
   RefCountable(const RefCountable& p) = delete;
   virtual ~RefCountable() { }

   void AddRef(void)  { refcnt_inc(&ref); }
   bool Release(void) { bool r; if ((r=(refcnt_dec(&ref)?true:false))) delete this; return r; }
};

template<typename T>
class Pointer
{
   T *ptr;

   void
   AddRef()
   {
      if (ptr)
         ptr->AddRef();
   }

   void
   Release()
   {
      if (ptr)
      {
         ptr->Release();
         ptr = nullptr;
      }
   }

public:

   Pointer() : ptr(nullptr) {}
   ~Pointer() { Release(); }
   Pointer(const Pointer<T> &other) : ptr(other.ptr) { AddRef(); }
   Pointer(Pointer<T> &&other) : ptr(other.ptr) { other.ptr = nullptr; }

   Pointer(T* p) : ptr(p) { AddRef(); }

   T *operator->() const        { return ptr; }
   T *Get() const               { return ptr; }
   T **GetAddressOf()           { return &ptr; }
   T **ReleaseAndGetAddressOf() { Release(); return GetAddressOf(); }

   T *Detach()
   {
      T *r = ptr;
      ptr = nullptr;
      return r;
   }

   void Attach(T* ptr)
   {
      Release();
      this->ptr = ptr;
   }

   Pointer<T> & operator =(T* ptr)
   {
      if (this->ptr != ptr)
      {
         Release();
         this->ptr = ptr;
         AddRef();
      }
      return *this;
   }

   Pointer<T> & operator =(const Pointer<T> &ptr)
   {
      return *this = ptr.ptr;
   }

   Pointer<T> & operator =(Pointer<T> &&ptr)
   {
      if (&ptr != this)
      {
         Release();
         this->ptr = ptr.ptr;
         ptr.ptr = nullptr;
      }
      return *this;
   }
};

} // namespace

#endif
