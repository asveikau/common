/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_cxx_lock_h
#define common_cxx_lock_h

#include <common/thread.h>
#include <functional>
#include <mutex>
#include <version>
#if defined(__cpp_lib_shared_mutex)
#include <shared_mutex>
#endif
#include <new>

namespace common {

class locker
{
   void *context;
   void (*unlock)(void*);
public:
   locker() : context(nullptr) {}
   locker(const locker &) = delete;
   ~locker() { release(); }

   void acquire(void *context, void(*unlock)(void*))
   {
      this->context = context;
      this->unlock = unlock;
   }

   void acquire(const std::function<void(void)> &fn, error *err)
   {
      try
      {
         acquire(
            new std::function<void(void)>(fn),
            [] (void*p) -> void
            {
               auto fn = reinterpret_cast<std::function<void(void)>*>(p);
               (*fn)();
               delete fn;
            }
         );
      }
      catch (std::bad_alloc)
      {
         fn();
         ERROR_SET(err, nomem);
      }
   exit:;
   }

   void acquire(mutex &m) { acquire(&m); }
   void acquire(mutex *m)
   {
      release();
      mutex_acquire(m);
      acquire(m, [] (void*p) -> void { mutex_release((mutex*)p); });
   }

   void acquire(std::mutex &m)
   {
      release();
      m.lock();
      acquire(&m, [] (void* p) -> void
      {
         auto &q = *(std::mutex*)p;
         q.unlock();
      });
   }

#if defined(__cpp_lib_shared_mutex)
   void acquire(std::shared_mutex &m)
   {
      release();
      m.lock();
      acquire(&m, [] (void* p) -> void
      {
         auto &q = *(std::shared_mutex*)p;
         q.unlock();
      });
   }

   void acquire_shared(std::shared_mutex &m)
   {
      release();
      m.lock_shared();
      acquire(&m, [] (void* p) -> void
      {
         auto &q = *(std::shared_mutex*)p;
         q.unlock_shared();
      });
   }
#endif

#if defined(_WINDOWS)
   void acquire(PCRITICAL_SECTION lock)
   {
      release();
      EnterCriticalSection(lock);
      acquire(lock, [] (void*p) -> void { LeaveCriticalSection((PCRITICAL_SECTION)p); });
   }
#endif

   void release()
   {
      if (unlock && context)
      {
         unlock(context);
         unlock = nullptr;
         context = nullptr;
      }
   }
};

} // end namespace

#endif
