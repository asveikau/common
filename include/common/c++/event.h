/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_event_h_
#define common_event_h_

#include "../error.h"
#include <vector>
#include <functional>
#include <stdlib.h>
#include <string.h>

namespace common {

template<typename... T>
struct EventSubscribeTracker;

template <typename... T>
class Event
{
   struct node
   {
      std::function<void(T..., error *err)> fn;
      struct node *next;

      node(const std::function<void(T..., error*)> &fn_) : fn(fn_), next(nullptr)
      {
      }
   };
   struct node *head;
   struct node **tail;
   EventSubscribeTracker<T...> *tracker;
public:
   Event(EventSubscribeTracker<T...> *tracker_=nullptr)
      : head(nullptr), tail(&head), tracker(tracker_)
   {
      if (tracker)
         tracker->event = this;
   }
   Event(const Event& p) = delete;
   ~Event()
   {
      auto p = head;
      while (p)
      {
         auto q = p->next;
         delete p;
         p = q;
      }
   }

   void Invoke(T... p)
   {
      Invoke(p..., nullptr);
   }

   void Invoke(T... p, error *err)
   {
      error errStorage;

      if (!err)
      {
         err = &errStorage;
      }

      auto q = head;
      while (q)
      {
         auto r = q->next;
         q->fn(p..., err);
         ERROR_CHECK(err);
         q = r;
      }

   exit:;
   }

   typedef std::function<void()> UnsubscribeFunc;

   UnsubscribeFunc Subscribe(const std::function<void(T...)> &cb, error *err)
   {
      return Subscribe([cb] (T... arg, error *ignored) -> void { cb(arg...); }, err);
   }

   UnsubscribeFunc Subscribe(const std::function<void(T..., error*)> &cb, error *err)
   {
      node *p = nullptr;
      bool tracked = false;

      try
      {
         p = new node(cb);
      }
      catch (std::bad_alloc)
      {
         ERROR_SET(err, nomem);
      }

      if (tracker && !(tracker->refCount++))
      {
         tracked = true;
         tracker->OnSubscribe(err);
         ERROR_CHECK(err);
      }

      *tail = p;
      tail = &p->next;
   exit:
      if (ERROR_FAILED(err))
      {
         if (p) delete p;
         if (tracked)
            --tracker->refCount;
         return UnsubscribeFunc();
      }
      return [this, p] () mutable -> void
      {
         if (!p)
            return;

         node *prev = nullptr;
         auto q = head;

         while (q && q != p)  
         {
            prev = q;
            q = q->next;
         }

         if (p != q)
            return;

         if (prev)
            prev->next = p->next;
         else
            head = p->next;

         if (tail == &p->next)
            tail = prev ? &prev->next : &head;

         delete p;
         p = nullptr;
         if (tracker && !(--tracker->refCount))
         {
            error err;
            tracker->OnUnsubscribe(&err);
         }
      };
   }

   bool HasSubscribers(void) const { return !!head; }
};

template<typename... T>
struct EventSubscribeTracker
{
   Event<T...> *event;
   int refCount;
   EventSubscribeTracker() : event(nullptr), refCount(0) {}
   EventSubscribeTracker(const EventSubscribeTracker &p) = delete;
   virtual ~EventSubscribeTracker()
   {
      if (refCount) 
      {
         error err;
         OnUnsubscribe(&err);
      }
   }
   virtual void OnSubscribe(error *err) = 0;
   virtual void OnUnsubscribe(error *err) {}

   void Callback(T... args, error *err)
   {
      if (event)
         event->Invoke(args..., err);
   } 
};

template<typename T, typename... InnerT>
struct EventSubscribeTrackerWithState : public EventSubscribeTracker<InnerT...>
{
   bool hasState;
protected:
   T state;

   virtual void InnerSubscribe(error *err) = 0;
   virtual void InnerUnsubscribe(error *err) {}
public:
   EventSubscribeTrackerWithState() : hasState(false) {}

   void OnAvailable(T st, error *err)
   {
      state = std::move(st);
      hasState = true;
      if (this->refCount)
         InnerSubscribe(err);
   }

   void Detach(error *err)
   {
      if (hasState)
         InnerUnsubscribe(err);
      hasState = false;
   }

   void OnSubscribe(error *err)
   {
      if (hasState)
         InnerSubscribe(err);
   }
   void OnUnsubscribe(error *err)
   {
      if (hasState)
         InnerUnsubscribe(err);
   }
};

} // end namespace

#endif
