/*
 Copyright (C) 2017-2019 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/c++/refcount.h>
#include <common/c++/lock.h>
#include <common/cas.h>
#include <string.h>

namespace common { namespace internal {
struct WeakPointerControlBlock
{
   refcnt ref;
   RefCountable * volatile ptr;
#if defined(__cpp_lib_shared_mutex)
   std::shared_mutex lock;
#else
   rwlock lock;
#endif

   WeakPointerControlBlock() : ref(1), ptr(nullptr)
   {
#if !defined(__cpp_lib_shared_mutex)
      memset(&lock, 0, sizeof(lock));
#endif
   }

   ~WeakPointerControlBlock()
   {
#if !defined(__cpp_lib_shared_mutex)
      rwlock_destroy(&lock);
#endif
   }

   void
   Initialize(error *err)
   {
#if !defined(__cpp_lib_shared_mutex)
      rwlock_init(&lock, err);
#endif
   }

   void
   AcquireExclusive(locker &l)
   {
      l.acquire(lock);
   }

   void
   AcquireShared(locker &l)
   {
      l.acquire_shared(lock);
   }

   void
   AddRef()
   {
      refcnt_inc(&ref);
   }

   void
   Release()
   {
      if (refcnt_dec(&ref))
         delete this;
   }

   RefCountable *
   Lock()
   {
      locker l;
      AcquireExclusive(l);
      if (ptr)
         ptr->AddRef();
      return ptr;
   }
};
} } // end namespace

common::internal::WeakPointerControlBlock *
common::RefCountable::MakeWeakImpl(error *err)
{
   internal::WeakPointerControlBlock *r = nullptr;

retry:
   if (ERROR_FAILED(err))
      goto exit;
   if ((r = wpcb))
      goto exit;

   try
   {
      r = new internal::WeakPointerControlBlock();
   }
   catch (std::bad_alloc)
   {
      ERROR_SET(err, nomem);
   }
   r->ptr = this;
   r->Initialize(err);
   ERROR_CHECK(err);

   if (!compare_and_swap_pointer((void * volatile *)&wpcb, nullptr, r))
   {
      r->Release();
      r = nullptr;
      goto retry;
   }
exit:
   if (ERROR_FAILED(err))
   {
      if (r)
      {
         r->Release();
         r = nullptr;
      }
   }
   else
   {
      r->AddRef();
   }
   return r;
}

common::RefCountable::RefCountable()
   : ref(1), wpcb(nullptr)
{
}

common::RefCountable::~RefCountable()
{
   if (wpcb)
   {
      wpcb->Release();
      wpcb = nullptr;
   }
}

void
common::RefCountable::AddRef(void)
{
   refcnt_inc(&ref);
}

bool
common::RefCountable::Release(void)
{
   bool r = false;
   auto wpcb = this->wpcb;
   locker l;
   if (wpcb)
      wpcb->AcquireShared(l);
   if ((r=(refcnt_dec(&ref)?true:false)))
   {
      if (wpcb)
      {
         wpcb->ptr = nullptr;
         l.release();
      }
      delete this;
   }
   return r;
}

void
common::internal::WeakPointerBase::AddRef()
{
   if (wpcb)
      wpcb->AddRef();
}

void
common::internal::WeakPointerBase::Release()
{
   if (wpcb)
   {
      wpcb->Release();
      wpcb = nullptr;
   }
}

common::RefCountable *
common::internal::WeakPointerBase::Lock()
{
   return wpcb ? wpcb->Lock() : nullptr;
}
