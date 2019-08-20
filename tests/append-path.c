#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common/logger.h>
#include <common/path.h>

static
void
log_cb(void *ctx, const char *msg)
{
   fputs(msg, stderr);
}

int
main()
{
   log_register_callback(log_cb, NULL);
   error err = {0};
   int r = 0;
   char *res = NULL;

   struct test_case
   {
      const char *prefix, *suffix, *expected;
   };
   static const struct test_case cases[] =
   {
      {"/",     "foo",                  "/foo"},
      {"/",     "/foo",                 "/foo"},
      {"/",     "foo/bar/../baz",       "/foo/baz"},
      {"/",     "/foo/bar/../baz",      "/foo/baz"},
      {"/",     ".",                    "/"},
      {"/",     "..",                   "/"},
      {"foo",   ".",                    "foo"},
      {"/foo",  ".",                    "/foo"},
      {"foo",   "..",                   "."},
      {"/foo",  "..",                   "/"},
      {"foo",   "../bar",               "bar"},
      {"/foo",  "../bar",               "/bar"},
      {"foo",   "../..",                ".."},
      {"/usr",  "bin",                  "/usr/bin"},
      {"/usr/", "bin/",                 "/usr/bin"},
      {"/usr/", "/local///share/bin/",  "/usr/local/share/bin"},
      {NULL, NULL, NULL}
   };
   const struct test_case *p = cases;

   for (; p->prefix; ++p)
   {
      free(res);
      res = append_path(p->prefix, p->suffix, &err);
      ERROR_CHECK(&err);

      if (!res || strcmp(res, p->expected))
      {
         static char msg[4096];

         snprintf(
            msg, sizeof(msg),
            "[%s] + [%s] = %s%s%s, expected %s",
            p->prefix,
            p->suffix,
            res ? "[" : "",
            res ? res : "NULL",
            res ? "]" : "",
            p->expected
         );

         ERROR_SET(&err, unknown, msg);
      }
   }

exit:
   free(res);
   r = ERROR_FAILED(&err) ? 1 : 0;
   error_clear(&err);
   return r;
}
