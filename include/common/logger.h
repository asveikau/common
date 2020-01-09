/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef logger_h_
#define logger_h_

#include <stdarg.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif 

void
log_printf(const char *fmt, ...);

void
log_vprintf(const char *fmt, va_list ap);

typedef
void
(*logger_callback_fn)(void *, const char *msg);

int
log_register_callback(logger_callback_fn fn, void *context);

void
log_unregister_callback(int id);

void
log_register_default_callback();

bool
log_unregister_default_callback();

void
register_backtrace_logger(
   void (*on_crash)(void*),  // can be null
   void *ctx
);

#if defined(__cplusplus)
}
#endif
#endif
