#include <common/logger.h>
#include <common/path.h>

#include <stdio.h>
#include <stdlib.h>

static void
log_callback(void *context, const char *buffer)
{
   fprintf(stderr, "%s\n", buffer);
}

char *
get_mount_point(const char *path, error *err);

char *
get_fs_type(const char *path, error *err);

int main(int argc, char **argv)
{
   error err = {0};
   int r = 0;
   char *p = NULL;
   bool remote = false;

   log_register_callback(log_callback, NULL);

   if (!argv[1])
      ERROR_SET(&err, unknown, "usage: fsinfo path");

   printf("path:        %s\n", argv[1]);

   p = get_mount_point(argv[1], &err);
   ERROR_CHECK(&err);
   printf("mount point: %s\n", p);
   free(p);
   p = NULL;

   p = get_fs_type(argv[1], &err);
   ERROR_CHECK(&err);
   printf("fs type:     %s\n", p);
   free(p);
   p = NULL;

   remote = path_is_remote(argv[1], &err);
   ERROR_CHECK(&err);
   printf("remote?:     %s\n", remote ? "yes" : "no");

exit:
   free(p);
   r = ERROR_FAILED(&err) ? 1 : 0;
   error_clear(&err);
   return r;
}
