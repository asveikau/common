/*
 Copyright (C) 2019 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#if defined(XP_SUPPORT)

#define FOREACH_FUNC(FN) \
   FN(AcquireSRWLockShared); \
   FN(AcquireSRWLockExclusive); \
   FN(ReleaseSRWLockExclusive); \
   FN(ReleaseSRWLockShared);

#define DECLARE_FUNC(X) static void (WINAPI *X##_XpSup)(PSRWLOCK) = NULL
FOREACH_FUNC(DECLARE_FUNC)

static
bool
PostXpSupportLoaded()
{
#define CHECK_NULL(X) if (!X##_XpSup) return false;
   FOREACH_FUNC(CHECK_NULL);
   return true;
}

static void
TryLoadPostXpSupport()
{
   if (PostXpSupportLoaded())
      return;

   HMODULE mod = GetModuleHandle(L"kernelbase.dll");
   if (!mod)
      mod = GetModuleHandle(L"kernel32.dll");
   if (!mod)
      return;

#define LOAD_FUNC(FN) (FN##_XpSup = (void (WINAPI*)(PSRWLOCK))GetProcAddress(mod, #FN))
   FOREACH_FUNC(LOAD_FUNC);
}

#define AcquireSRWLockShared    AcquireSRWLockShared_XpSup
#define AcquireSRWLockExclusive AcquireSRWLockExclusive_XpSup
#define ReleaseSRWLockExclusive ReleaseSRWLockExclusive_XpSup
#define ReleaseSRWLockShared    ReleaseSRWLockShared_XpSup
#endif