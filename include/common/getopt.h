/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_getopt_h_
#define common_getopt_h_

#if defined(_WINDOWS)
#if defined(__cplusplus)
extern "C" {
#endif

extern char *optarg;
extern int optind, optopt, opterr, optreset;

int
getopt(int argc, char * const argv[], const char *optstring);

#if defined(__cplusplus)
}
#endif
#else
#include <unistd.h>
#endif

#endif
