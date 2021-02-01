/*
 Copyright (C) 2019 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/c++/dtorqueue.h>

common::DestructorQueue::DestructorQueue()
{
}

common::DestructorQueue::~DestructorQueue()
{
   Destroy();
}

void
common::DestructorQueue::Add(std::function<void(void)> &fn, error *err)
{
   if (ERROR_FAILED(err))
      goto exit;
   try
   {
      dtors.push_back(std::move(fn));
   }
   catch (const std::bad_alloc&)
   {
      ERROR_SET(err, nomem);
   }
exit:
   if (ERROR_FAILED(err))
      fn();
   fn = std::function<void(void)>();
}

void
common::DestructorQueue::Destroy()
{
   auto dtors = std::move(this->dtors);
   this->dtors = std::vector<std::function<void(void)>>();
   for (auto &fn : dtors)
   {
      fn();
   }
}