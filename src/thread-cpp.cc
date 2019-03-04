/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/thread.h>
#include <errno.h>

typedef std::function<void(void)> ArgType;

namespace {

extern "C"
THREAD_PROC_RETVAL
ThreadProc(void *arg)
{
   auto fn = (ArgType*)arg;
   (*fn)();
   delete fn;
   return 0;
}

} // end namespace

void
common::create_thread(
   std::function<void(void)> fn,
   thread_id *idOut,
   error *err
)
{
   ArgType *p = nullptr;
   try
   {
      p = new ArgType(fn);
   }
   catch (std::bad_alloc)
   {
      ERROR_SET(err, nomem);
   }
   ::create_thread(p, ThreadProc, idOut, err);
   ERROR_CHECK(err);
   p = nullptr;
exit:
   if (p) delete p;
}
