/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/lazy.h>
#include <common/cas.h>

enum
{
   None = 0,
   Initializing,
   Initialized,
};

#if defined(_WINDOWS)
#define spin() Sleep(0)
#elif defined(__linux__)
#include <sched.h>
#define spin() sched_yield()
#else
#include <unistd.h>
#define spin() usleep(800)
#endif 

void
lazy_init(
  lazy_init_state *state,
  void (*fn)(void *context, error *err),
  void *context,
  error *err
)
{
  for (;;)
  {
     switch (state->state)
     {
        case Initialized:
           return;
        case None:
           if (compare_and_swap(&state->state, None, Initializing))
           {
              fn(context, err);
              state->state = (err && ERROR_FAILED(err)) ? None : Initialized;
              memory_barrier();
              return;
           } 
        case Initializing:
           spin();
           continue;
        default:
           if (err)
              error_set_unknown(err, "Unexpected enum value");
           return;
     }
  }
}

bool
lazy_is_initialized(lazy_init_state *state)
{
   return state->state == Initialized;
}