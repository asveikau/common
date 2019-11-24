/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/c++/worker.h>
#include <common/c++/lock.h>
#include <string.h>

using namespace common;

struct common::WorkerThreadBase::SlowPathQueueNode
{
   std::function<void(void)> fn;
   SlowPathQueueNode *next;
};

void
common::WorkerThreadBase::Drain(std::function<void(void)> *funcs, int nfuncs)
{
   int r = 0;

   while ((r = queue.Read(funcs, nfuncs)))
   {
   retry:
      for (int i=0; i<r; ++i)
      {
         auto &func = funcs[i];
         func();
         funcs[i] = nullptr;
      }
   }

   if (slowQueueHead)
   {
      locker lock;
      lock.acquire(producerMutex);
      r = queue.Read(funcs, nfuncs);
      int n = nfuncs - r;
      auto p = funcs + r;
      while (n && slowQueueHead)
      {
         *p++ = std::move(slowQueueHead->fn);
         ++r;
         --n;
         SlowQueueChomp();
      }
   }

   if (r) goto retry;

   if (localSignal)
   {
      // The drain triggered another enqueue, don't bother
      // going to the semaphore yet, just drain again.
      //
      localSignal = false;
      goto retry;
   }
}

void
common::WorkerThreadBase::SlowQueueChomp(void)
{
   auto p = slowQueueHead;
   if (p)
   {
      slowQueueHead = p->next;
      if (&p->next == slowQueueTail)
         slowQueueTail = &slowQueueHead;
      delete p;
   }
}

void
common::WorkerThreadBase::Write(std::function<void(void)> &fn, error *err)
{
   locker lock;
   lock.acquire(producerMutex);
   while (slowQueueHead && queue.Write(&slowQueueHead->fn, 1))
      SlowQueueChomp();
   if (slowQueueHead || !queue.Write(&fn, 1))
   {
      auto p = new (std::nothrow) SlowPathQueueNode();
      if (!p)
         ERROR_SET(err, nomem);
      p->fn = std::move(fn); 
      p->next = nullptr;
      *slowQueueTail = p;
      slowQueueTail = &p->next;
   }
exit:;
}

std::function<void(void)>
common::WorkerThreadBase::PrepFunction(
   const std::function<void(error*)> &fn,
   error *asyncErr
)
{
   if (asyncErr)
      return [fn, asyncErr] () -> void { fn(asyncErr); };
   else
      return [fn] () -> void
             {
                error err;
                fn(&err);
             };
}

void
common::WorkerThreadBase::ScheduleImpl(
   const std::function<void(error *err)> &fn,
   bool synchronous,
   error *err,
   error *asyncErr
)
{
   error defaultErrorStorage;

   if (!err)
      err = &defaultErrorStorage;

   if (IsOnThread())
   {
      if (synchronous)
         fn(err);
      else
      {
         auto inner = PrepFunction(fn, asyncErr);
         Write(inner, err);
         ERROR_CHECK(err);
         localSignal = true;
      }
   }
   else
   {
      if (synchronous)
      {
         ScheduleSyncViaAsync(
            fn,
            [this] (const std::function<void(error*)> &fn, error *err) -> void
            {
               Schedule(fn, false, err, err);
            },
            err
         );
      }
      else
      {
         auto inner = PrepFunction(fn, asyncErr);
         Write(inner, err);
         ERROR_CHECK(err);
         Signal(err);
         ERROR_CHECK(err);
      }
   }

exit:;
}

bool
common::WorkerThreadBase::IsEmpty(void)
{
   return queue.IsEmpty() && !slowQueueHead;
}

common::WorkerThreadBase::WorkerThreadBase()
   : localSignal(false),
     slowQueueHead(nullptr),
     slowQueueTail(&slowQueueHead)
{
   error err;

   memset(&producerMutex, 0, sizeof(producerMutex));

   mutex_init(&producerMutex, &err);
   ERROR_CHECK(&err);

exit:
   if (ERROR_FAILED(&err))
   {
      Cleanup();
      throw std::bad_alloc();
   }
}

common::WorkerThreadBase::~WorkerThreadBase()
{
   Cleanup();
}

void
common::WorkerThreadBase::Cleanup()
{
   mutex_destroy(&producerMutex);
}

//
//
//

common::WorkerThread::WorkerThread()
   : stopping(false)
{
   error err;

   memset(&thread, 0, sizeof(thread));
   memset(&consumerSem, 0, sizeof(consumerSem));

   sm_init(&consumerSem, 0, &err);
   ERROR_CHECK(&err);

   create_thread(
      [this] () -> void
      {
         std::function<void(void)> funcs[256];

         for (;;)
         {
            Drain(funcs, ARRAY_SIZE(funcs));
            if (stopping && IsEmpty())
               break;
            sm_wait(&consumerSem);
         }
      },
      &thread,
      &err
   );
   ERROR_CHECK(&err);

exit:
   if (ERROR_FAILED(&err))
   {
      Cleanup();
      throw std::bad_alloc();
   }
}

common::WorkerThread::~WorkerThread()
{
   Cleanup();
}

void
common::WorkerThread::Cleanup(void)
{
   if (thread_is_started(&thread))
   {
      stopping = true;
      sm_post(&consumerSem);
      join_thread(&thread);
   }
   sm_destroy(&consumerSem);
}

void
common::WorkerThread::Signal(error *err)
{
   sm_post(&consumerSem);
}

bool
common::WorkerThread::IsOnThread(void)
{
   return thread_is_self(&thread);
}
