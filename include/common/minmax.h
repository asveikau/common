/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef libcommon_minmax_h
#define libcommon_minmax_h

#ifndef MIN
#define MIN(X, Y)    (((X) < (Y)) ? (X) : (Y))
#endif

#ifndef MAX
#define MAX(X, Y)    (((X) > (Y)) ? (X) : (Y))
#endif

#endif