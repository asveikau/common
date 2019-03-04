/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/path.h>
#include <stdlib.h>
#include <string.h>

static char *bundle = NULL;

#if defined(__APPLE__)
#define HAVE_PLATFORM_SPECIFIC
#endif

#ifdef HAVE_PLATFORM_SPECIFIC
extern
const char *
get_bundle_path_platform_specific();

#include <common/lazy.h>

static void
get_bundle_lazy_impl(void *ctx, error *err)
{
   const char *p = get_bundle_path_platform_specific();
   if (p)
      libcommon_set_bundle_path(p, err);
}

static void
get_bundle_lazy()
{
   error err;
   static lazy_init_state lazy = {0};
   memset(&err, 0 ,sizeof(err));
   lazy_init(&lazy, get_bundle_lazy_impl, NULL, &err);
   error_clear(&err);
}
#endif

const char *
get_bundle_path()
{
#ifdef HAVE_PLATFORM_SPECIFIC
   if (!bundle)
      get_bundle_lazy();
#endif

   return bundle;
}

void
libcommon_set_bundle_path(const char *path, error *err)
{
   char *heap = NULL;
   size_t len = 0;

   if (!path || !*path)
   {
      heap = bundle;
      bundle = NULL;
      goto exit;
   }

   len = strlen(path) + 1;
   heap = malloc(len);
   if (!heap)
      ERROR_SET(err, nomem);

   memcpy(heap, path, len);

exit:
   if (!ERROR_FAILED(err) && heap && path && *path)
   {
      char *p = bundle;
      bundle = heap;
      heap = p;
   }
   free(heap);
}
