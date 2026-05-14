/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef libcommon_fieldoffset_h
#define libcommon_fieldoffset_h

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, memb) ((intptr_t)(&((type*)0)->memb))
#endif

#endif
