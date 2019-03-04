/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_cxx_registrationlist_h_
#define common_cxx_registrationlist_h_

#include <mutex>
#include <new>

#include "refcount.h"
#include "stream.h"

namespace common
{

template<typename T>
class RegistrationList
{
public:

   RegistrationList() : list(nullptr) {}
   RegistrationList(const RegistrationList &p) = delete;
   ~RegistrationList()
   {
      auto p = list;
      while (p)
      {
         auto q = p->Next;
         delete p;
         p = q;
      }
   }

   struct RegisteredItem
   {
      Pointer<T> Item;
      RegisteredItem *Next;
   };

   void
   Register(
      T *item,
      error *err
   )
   {
      auto p = new(std::nothrow) RegisteredItem();
      if (!p)
      {
         ERROR_SET(err, nomem);
      exit:
         return;
      }
      p->Item = item;

      std::lock_guard<std::mutex> guard(lock);
      p->Next = list;
      list = p;
   }

   template <typename U, typename V>
   void
   ForEach(U cb, V cont, error *err)
   {
      auto p = list;
      while (p && cont())
      {
         cb(p->Item.Get(), err);
         ERROR_CHECK(err);
         p = p->Next;
      }
   exit:;
   }

   template <typename U>
   void
   ForEach(U cb, error *err)
   {
      ForEach(cb, [] () -> bool { return true; }, err);
   }

   template <typename U, typename V>
   void
   TryLoad(common::Stream *file, U **r, V cb, error *err)
   {
      common::Pointer<U> newObject;

      auto origin = file->GetPosition(err);
      ERROR_CHECK(err);

      ForEach(
         [&] (T *item, error *err) -> void
         {
            cb(item, newObject.GetAddressOf(), err);

            if (!ERROR_FAILED(err) && newObject.Get())
               goto exit;

            // Seek back to start of file in case the codec touched
            // the stream.
            //
            error_clear(err);
            file->Seek(origin, SEEK_SET, err);
            error_clear(err);
            newObject = nullptr;
         exit:;
         },
         [&] () -> bool { return !newObject.Get(); },
         err
      );
      ERROR_CHECK(err);
   exit:
      if (ERROR_FAILED(err))
         newObject = nullptr;
      *r = newObject.Detach();
   }

   bool
   HasItems(void) const
   {
      return list ? true : false;
   }

private:
   std::mutex lock;
   RegisteredItem *list;
};

} // end namespace

#endif
