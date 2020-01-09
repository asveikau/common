#include <common/logger.h>
#include <common/path.h>

#include <stdio.h>

static
void
log_cb(void *ctx, const char *msg)
{
   fputs(msg, stderr);
}

int
main(int argc, char **argv)
{
   log_register_callback(log_cb, NULL);
   error err = {0};
   int r = 0;

   if (argc != 3)
      ERROR_SET(&err, unknown, "Usage: cp src dst");

   copy_file(argv[1], argv[2], &err);
   ERROR_CHECK(&err);

exit:
   r = ERROR_FAILED(&err) ? 1 : 0;
   error_clear(&err);
   return r;
}
