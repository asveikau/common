/*
 Copyright (C) 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/trie.h>
#include <common/misc.h>

#include <stdlib.h>
#include <string.h>

static int
check_allocation(struct trie **triep)
{
   if (!*triep)
   {
      struct trie *trie = malloc(sizeof(*trie));
      if (!trie)
         return -1;
      memset(trie, 0, sizeof(*trie));
      *triep = trie;
   }
   return 0;
}

void
trie_insert(
   struct trie **trie,
   const void *keyp,
   size_t keylen,
   void *value,
   void (*dtor)(void*),
   error *err
)
{
   const unsigned char *key = keyp;

   while (keylen)
   {
      if (check_allocation(trie))
         ERROR_SET(err, nomem);
      trie = &(*trie)->subtries[*key++];
      --keylen;
   }

   if (check_allocation(trie))
      ERROR_SET(err, nomem);

   if ((*trie)->dtor)
      (*trie)->dtor((*trie)->value);

   (*trie)->value = value;
   (*trie)->dtor = dtor;

exit:;
}

void *
trie_find(
   struct trie *trie,
   const void *keyp,
   size_t keylen
)
{
   const unsigned char *key = keyp;
   void *r = NULL;

   while (trie && keylen)
   {
      trie = trie->subtries[*key++];
      --keylen;
   }

   if (trie && !keylen)
      r = trie->value;

   return r;
}

static void
trie_do_remove(
   struct trie **trie,
   const unsigned char *key,
   size_t keylen,
   int *found
)
{
   struct trie *t = *trie;
   int i = 0;

   if (t && !keylen)
   {
      if (t->dtor)
         t->dtor(t->value);
      t->dtor = NULL;
      t->value = NULL;
      *found = 1;
      *trie = NULL;
   }
   else if (keylen)
   {
      trie_do_remove(&t->subtries[*key], key + 1, keylen - 1, found);
   }

   if (!*found || !t || t->value)
      return;

   for (i=0; i<256; ++i)
   {
      if (t->subtries[i])
         return;
   }

   *trie = NULL;
   trie_free(t);
}

void
trie_remove(
   struct trie **trie,
   const void *keyp,
   size_t keylen
)
{
   int found = 0;
   trie_do_remove(trie, keyp, keylen, &found);
}

void
trie_free(
   struct trie *trie
)
{
   if (trie)
   {
      struct trie **p = trie->subtries, **e = p + ARRAY_SIZE(trie->subtries);
      while (p < e)
         trie_free(*p++);
      if (trie->dtor)
         trie->dtor(trie->value);
      free(trie);
   }
}

