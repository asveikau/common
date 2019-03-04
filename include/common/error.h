/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef error_h_
#define error_h_

#include <string.h>
#include <errno.h>

#if defined(_WINDOWS)
#include <windows.h>
#endif
#if defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum
{
   ERROR_SRC_SUCCESS = 0,
   ERROR_SRC_ERRNO,
   ERROR_SRC_COM,
   ERROR_SRC_UNKNOWN,
   ERROR_SRC_OSSTATUS,
   ERROR_SRC_DARWIN,    // Mach / IOKit
} error_source;

#if defined(__cplusplus)
void error_clear(struct error_ *);
#endif

typedef struct error_
{
   error_source source;
   int code;
   void *context;
   const char *(*get_string)(struct error_ *err);
   void (*free_fn)(void* context);

#if defined(__cplusplus)
   error_() { memset(this, 0, sizeof(*this)); }
   error_(const error_&p) = delete;
   ~error_() { error_clear(this); }
#endif
} error;

const char*
error_get_string(error *);

void
error_set_errno(error *err, int errno_value);

void
error_set_unknown(error *, const char *msg);

void
error_set_nomem(error *);

#if defined(_WINDOWS)
void
error_set_win32(error *err, DWORD code);

void
error_set_hresult(error *err, HRESULT hr);

void
error_set_ntstatus(error *err, NTSTATUS status);
#endif

#if defined(__APPLE__)
void
error_set_osstatus(error *err, OSStatus oserr);

void
error_set_darwin(error *err, int ret);
#endif

void
error_log(error *err, const char *func, const char *file, int line_no);

void
error_clear(error *);

#if defined(__cplusplus) && defined(__GNUC__)
#define ERROR_FUNCTION_MACRO __PRETTY_FUNCTION__
#else
#define ERROR_FUNCTION_MACRO __FUNCTION__
#endif

#ifndef ERROR_JMP
#define ERROR_JMP() goto exit
#endif

#define ERROR_FAILED(ERR) ((ERR)->source ? 1 : 0)

#define ERROR_CHECK(err) \
   do                    \
   {                     \
      if ((err)->source) \
         ERROR_JMP();    \
   } while (0)

#define ERROR_LOG(err) \
   error_log(err, ERROR_FUNCTION_MACRO, __FILE__, __LINE__)

#define ERROR_SET(err, type, ...)           \
   do                                       \
   {                                        \
      error_set_##type(err, ##__VA_ARGS__); \
      ERROR_LOG(err);                       \
      ERROR_JMP();                          \
   } while (0)

#define ERROR_SET_NZ(err, type, expr, ...)  \
   do                                       \
   {                                        \
      if ((expr))                           \
         ERROR_SET(err, type, __VA_ARGS__); \
   } while(0)

#define ERROR_SET_BOOL(err, type, expr, ...) \
   ERROR_SET_NZ(err, type, !(expr), __VA_ARGS__)

#if defined(__cplusplus)
#define ERROR_SET_VALUE_BODY(expr) \
     auto tmp_expr_ = (expr)
#elif defined(__GNUC__)
#define ERROR_SET_VALUE_BODY(expr) \
     __auto_type tmp_expr_ = (expr)
#else
#define ERROR_SET_VALUE_BODY(expr) \
     int tmp_expr_ = (expr) // XXX
#endif

#define ERROR_SET_VALUE(err, type, expr)  \
   do                                     \
   {                                      \
      ERROR_SET_VALUE_BODY(expr);         \
      if ((tmp_expr_))                    \
         ERROR_SET(err, type, tmp_expr_); \
   } while(0) 

#if defined(__cplusplus)
}
#endif
#endif
