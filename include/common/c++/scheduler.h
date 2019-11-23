/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_sched_h_
#define common_sched_h_

#include "../error.h"
#include <functional>

namespace common {

struct Scheduler
{
   Scheduler() {}
   Scheduler(const Scheduler &p) = delete;
   virtual ~Scheduler();

   void
   Schedule(
      std::function<void(error*)> func,
      bool synchronous,
      error *err,
      error *asyncErr = nullptr
   )
   {
      ScheduleImpl(func, false, err, asyncErr);
   }

   void
   Schedule(
      std::function<void(error*)> func,
      error *err = nullptr,
      error *asyncErr = nullptr
   )
   {
      ScheduleImpl(func, false, err, asyncErr);
   }

   static void
   ScheduleSyncViaAsync(
      std::function<void(error*)> func,
      std::function<void(std::function<void(error*)>, error *)> schedule,
      error *err
   );

protected:

   virtual void
   ScheduleImpl(
      std::function<void(error*)> func,
      bool synchronous,
      error *err,
      error *asyncErr = nullptr
   ) = 0;

};

} // end namespace

#endif
