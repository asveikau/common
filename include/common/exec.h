/*
 Copyright (C) Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_exec_h
#define common_exec_h

#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

enum popen_exec_flags
{
   POPEN_SEARCH_PATH = (1<<0),
   POPEN_CLOSE_FD0   = (1<<1),
   POPEN_CLOSE_FD1   = (1<<2),
   POPEN_CLOSE_FD2   = (1<<3),
};

struct popen_exec_args
{
   char *const *argv;
   int argc;

   enum popen_exec_flags flags;

   // Optional.
   //
   char *const *environ;

   // These may be set to null if not interesting.
   // Setting error to the same address as output is like dup2(2, 1).
   //
   FILE **input_pipe;
   FILE **output_pipe;
   FILE **error_pipe;
};

int
popen_exec(
   struct popen_exec_args *args
);

#if defined(__cplusplus)
}
#endif
#endif
