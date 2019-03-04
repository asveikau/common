/*
 Copyright (C) 2017, 2018 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/sem.h>

#include <assert.h>

void
sm_init(semaphore *sem, int initial, error *err)
{
#if defined(MUTEX_WINDOWS)
   sem->Semaphore = CreateSemaphore(NULL, initial, 0x80, NULL);
   if (!sem->Semaphore)
      ERROR_SET(err, win32, GetLastError());
#elif defined(SEMAPHORE_MACH)
   kern_return_t r = semaphore_create(
      mach_task_self(),
      &sem->sem,
      SYNC_POLICY_FIFO,
      initial
   );
   if (r)
      ERROR_SET(err, darwin, r);
#elif defined(SEMAPHORE_LIBDISPATCH)
   sem->sem = dispatch_semaphore_create(initial);
   if (!sem->sem)
      ERROR_SET(err, unknown, "dispatch_semaphore_create failed");
#elif defined(MUTEX_PTHREAD)
   if (sem_init(&sem->sem, 0, initial))
      ERROR_SET(err, errno, errno);
#else
#error
#endif
exit:;
}

void
sm_destroy(semaphore *sem)
{
#if defined(MUTEX_WINDOWS)
   CloseHandle(sem->Semaphore);
#elif defined(SEMAPHORE_MACH)
   semaphore_destroy(mach_task_self(), sem->sem);
#elif defined(SEMAPHORE_LIBDISPATCH)
   if (sem->sem)
      dispatch_release(sem->sem);
#elif defined(MUTEX_PTHREAD)
   sem_destroy(&sem->sem);
#else
#error
#endif
}

void
sm_wait(semaphore *sem)
{
#if defined(MUTEX_WINDOWS)
   WaitForSingleObject(sem->Semaphore, INFINITE);
#elif defined(SEMAPHORE_MACH)
   semaphore_wait(sem->sem);
#elif defined(SEMAPHORE_LIBDISPATCH)
   dispatch_semaphore_wait(sem->sem, DISPATCH_TIME_FOREVER);
#elif defined(MUTEX_PTHREAD)
   int r = sem_wait(&sem->sem);
   assert(!r); // XXX
#else
#error
#endif
}

void
sm_post(semaphore *sem)
{
#if defined(MUTEX_WINDOWS)
   ReleaseSemaphore(sem->Semaphore, 1, NULL);
#elif defined(SEMAPHORE_MACH)
   semaphore_signal(sem->sem);
#elif defined(SEMAPHORE_LIBDISPATCH)
   dispatch_semaphore_signal(sem->sem);
#elif defined(MUTEX_PTHREAD)
   int r = sem_post(&sem->sem);
   assert(!r); // XXX
#else
#error
#endif
}

