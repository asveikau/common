/*
 Copyright (C) 2017 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <stdio.h>

#include <common/crypto/md5.h>
#include <common/crypto/sha1.h>
#include <common/crypto/sha256.h>
#include <common/misc.h>

#define CONCAT(X, Y)  CONCAT2(X, Y)
#define CONCAT2(A, B) A ## _ ## B

typedef struct CONCAT(HASH_ALGORITHM, state) hash_state;
#define hash_init CONCAT(HASH_ALGORITHM, init)
#define hash_data CONCAT(HASH_ALGORITHM, hash_data)
#define hash_final CONCAT(HASH_ALGORITHM, final)

int main()
{
   hash_state state;
   char buffer[4096];
   void *digest;
   size_t digestlen;
   unsigned char *p;
   int r;

   hash_init(&state);

   stdio_set_binary(stdin);

   while ((r = fread(buffer, 1, sizeof(buffer), stdin)))
   {
      hash_data(&state, buffer, r);
   } 

   if (ferror(stdin))
   {
      perror("stdin");
      return -1;
   }

   hash_final(&state, &digest, &digestlen);

   p = digest;
   while (digestlen--)
      printf("%.2x", *p++);
   puts("");

   return 0;
}
