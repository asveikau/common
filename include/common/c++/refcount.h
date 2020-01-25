/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_cpp_refcount_h
#define common_cpp_refcount_h

#include "../refcnt.h"
#include "../error.h"

namespace common {

template<typename T>
class Pointer;

template<typename T>
class WeakPointer;

namespace internal {
struct WeakPointerControlBlock;
} // end namespace

class RefCountable
{
   refcnt ref;
   internal::WeakPointerControlBlock *wpcb;

   template<typename T>
   friend class Pointer;

   internal::WeakPointerControlBlock *
   MakeWeakImpl(error *err);
public:
   RefCountable();
   RefCountable(const RefCountable& p) = delete;
   virtual ~RefCountable();

   void
   AddRef(void);

   bool
   Release(void);
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

   WeakPointer<T>
   MakeWeak(error *err)
   {
      WeakPointer<T> r;
      if (this->ptr)
      {
         r.wpcb = this->ptr->MakeWeakImpl(err);
      }
      return r;
   }
};

namespace internal
{

class WeakPointerBase
{
protected:
   internal::WeakPointerControlBlock *wpcb;

   void
   AddRef();

   void
   Release();

   RefCountable *
   Lock() const;

   WeakPointerBase(internal::WeakPointerControlBlock *wpcb_) : wpcb(wpcb_) {}
public:
   ~WeakPointerBase() { Release(); }
};

} // end namespace

template<typename T>
class WeakPointer : public internal::WeakPointerBase
{
   friend class Pointer<T>;
public:
   WeakPointer() : WeakPointerBase(nullptr) {}
   WeakPointer(const WeakPointer<T> &other) : WeakPointerBase(other.wpcb) { AddRef(); }
   WeakPointer(WeakPointer<T> &&other) : WeakPointerBase(other.wpcb) { other.wpcb = nullptr; }

   Pointer<T>
   Lock() const
   {
      Pointer<T> ptr;
      if (wpcb)
         *ptr.GetAddressOf() = (T*)WeakPointerBase::Lock();
      return ptr;
   }

   WeakPointer<T> & operator =(const WeakPointer<T> &ptr)
   {
      if (ptr.wpcb != wpcb)
      {
         Release();
         wpcb = ptr.wpcb;
         AddRef();
      }
      return *this;
   }

   WeakPointer<T> & operator =(WeakPointer<T> &&ptr)
   {
      if (&ptr != this)
      {
         Release();
         wpcb = ptr.wpcb;
         ptr.wpcb = nullptr;
      }
      return *this;
   }
};

} // namespace

#endif
