#include <utf.h>
#include <misc.h>

#include <stdio.h>
#include <string.h>

struct utf_test_case
{
   uint32_t unichar;
   const char *expected_utf8;
   const char *expected_utf16;
};

static struct utf_test_case
test_cases[] =
{
   {0x24,     "24",       "0024"},
   {0xa2,     "c2a2",     "00a2"},
   {0x20ac,   "e282ac",   "20ac"},
   {0x24b62,  "f0a4ada2", "d852df62"},
   {0x7a,     "7a",       "007a"},
   {0x6c34,   "e6b0b4",   "6c34"},
   {0x10000,  "f0908080", "d800dc00"},
   {0x1d11e,  "f09d849e", "d834dd1e"},
   {0x10fffd, "f48fbfbd", "dbffdffd"},
   {0,        "00",       "0000"}
};

static int
u16_to_hex(uint16_t *s, int n, char *output, int nout)
{
   if (n * 4 >= nout)
      return -1;

   while (n--)
   {
      snprintf(output, 5, "%.4x", *s++);
      output += 4;
   }

   return 0;
}

static int
u8_to_hex(void *pv, int n, char *output, int nout)
{
   unsigned char *p = pv;

   if (n * 2 >= nout)
      return -1;

   while (n--)
   {
      snprintf(output, 3, "%.2x", *p++);
      output += 2;
   }

   return 0;
}

int main()
{
   struct utf_test_case *test, *last;
   int r = 0;

   for (
      test = test_cases, last = test+ARRAY_SIZE(test_cases);
      test < last;
      ++test
   )
   {
      uint32_t unichar = test->unichar;
      char hex[80];
      uint16_t utf16[2];
      char utf8[4];
      int size = 0;
      const char *p;
      const uint16_t *q;

      printf("unichar:%x ", unichar);

      size = utf8_encode(unichar, utf8, ARRAY_SIZE(utf8));
      if (size <= 0 || size > ARRAY_SIZE(utf8))
      {
         fprintf(stderr, "utf8_encode on %x returned %d\n", unichar, size);
         return -1;
      }

      if (u8_to_hex(utf8, size, hex, sizeof(hex)))
      {
         fprintf(stderr, "failed to convert to hex\n");
         return -1;
      }

      if (strcmp(hex, test->expected_utf8))
      {
         fprintf(stderr, "utf-8 did not match!\n");
         return -1;
      }

      p = utf8;
      if (utf8_decode(&p) != unichar)
      {
         fprintf(stderr, "utf-8 decode failed!\n");
         return -1;
      }

      if (unichar && (p-utf8) != size)
      {
         fprintf(
            stderr,
            "utf-8 size mismatch: got %d, expected %d\n",
            (int)(p-utf8), size
         );
         return -1;
      }

      printf("utf8:%s ", hex);

      size = utf16_encode(unichar, utf16, ARRAY_SIZE(utf16));
      if (size <= 0 || size > ARRAY_SIZE(utf16))
      {
         fprintf(stderr, "utf16_encode on %x returned %d\n", unichar, size);
         return -1;
      }

      if (u16_to_hex(utf16, size, hex, sizeof(hex)))
      {
         fprintf(stderr, "failed to convert to hex\n");
         return -1;
      }

      if (strcmp(hex, test->expected_utf16))
      {
         fprintf(stderr, "utf-16 did not match!\n");
         return -1;
      }

      q = utf16;
      if (utf16_decode(&q) != unichar)
      {
         fprintf(stderr, "utf-16 decode failed!\n");
         return -1;
      }

      if (unichar && (q-utf16) != size)
      {
         fprintf(
            stderr,
            "utf-16 size mismatch: got %d, expected %d\n",
            (int)(q-utf16), size
         );
         return -1;
      }

      printf("utf16:%s ", hex);

      puts("");
   }

   return r;
}
