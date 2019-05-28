#include <common/logger.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
log_callback(void *context, const char *buffer)
{
   printf("%s\n", buffer);
}

int main()
{
   char buf[4097] = {0};
   int id = log_register_callback(log_callback, NULL);
   if (id < 0)
      abort();

   memset(buf, ' ', sizeof(buf)-1);
   strncpy(buf, "Message on heap", 15);
   log_printf("%s", buf);

   log_printf("Hello world");

   log_unregister_callback(id);

   log_printf("This message should not work.");

   return 0;
} 

