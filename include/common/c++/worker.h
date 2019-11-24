/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_worker_h_
#define common_worker_h_

#include "scheduler.h"
#include "ring.h"
#include "../thread.h"
#include "../sem.h"

namespace common
{

class WorkerThreadBase : public Scheduler
{
   RingBuffer<std::function<void(void)>> queue;
   mutex producerMutex;
   bool localSignal;

   struct SlowPathQueueNode;
   SlowPathQueueNode *slowQueueHead;
   SlowPathQueueNode **slowQueueTail;

   void Cleanup();

   std::function<void(void)>
   PrepFunction(const std::function<void(error*)> &fn, error *asyncErr);

   void Write(std::function<void(void)> &fn, error *err);
   void SlowQueueChomp();

protected:

   bool IsEmpty(void);
   void Drain(std::function<void(void)> *funcs, int nfuncs);

   virtual bool IsOnThread(void) = 0;
   virtual void Signal(error *err) = 0;

   void
   ScheduleImpl(
      const std::function<void(error*)> &func,
      bool synchronous,
      error *err,
      error *asyncErr
   );

public:
   WorkerThreadBase();
   ~WorkerThreadBase();
};

class WorkerThread : public WorkerThreadBase
{
   thread_id thread;
   semaphore consumerSem;
   volatile bool stopping;

   void Cleanup(void);

protected:

   bool IsOnThread(void);
   void Signal(error *err);

public:

   WorkerThread();
   ~WorkerThread();
};

} // end namespace

#endif
