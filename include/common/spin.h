/*
 Copyright (C) 2017, 2018, 2020 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_spin_h__
#define common_spin_h__

//
// NB:
// There was recent discussion in tech circles, especially from a one
// Mr. Linus Torvalds, about how yielding on a spinlock is a bad idea
// if you have a CPU-affinity-aware scheduler and multiple cores.  So
// perhaps we should conditionally do this for single core machines.
//

#if defined(_WINDOWS)
#include <windows.h>
#define spin() SwitchToThread()
#elif defined(__linux__) || defined(__FreeBSD__) || \
      defined(__NetBSD__) || defined(__OpenBSD__) || \
      defined(__sun__) || defined(__APPLE__)
#include <sched.h>
#define spin() sched_yield()
#else
#include <unistd.h>
#define spin() usleep(1)
#endif

#endif