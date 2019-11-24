/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/c++/scheduler.h>
#include <common/sem.h>

using namespace common;

common::Scheduler::~Scheduler()
{
}

void
common::Scheduler::ScheduleSyncViaAsync(
   const std::function<void(error*)> &fn,
   std::function<void(const std::function<void(error*)> &, error *)> scheduler,
   error *err
)
{
   semaphore sem;
   bool initSem = false;

   sm_init(&sem, 0, err);
   ERROR_CHECK(err);
   initSem = true;

   scheduler(
      [&sem, fn] (error *err) -> void
      {
         fn(err);
         sm_post(&sem);
      },
      err
   );
   ERROR_CHECK(err);

   sm_wait(&sem);
exit:
   if (initSem)
      sm_destroy(&sem);
}
