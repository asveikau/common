/*
 Copyright (C) 2019 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_dtorqueue_h__
#define common_dtorqueue_h__

#include <functional>
#include <vector>

#include <common/error.h>

namespace common
{

class DestructorQueue
{
   std::vector<std::function<void(void)>> dtors;
public:
   DestructorQueue();
   DestructorQueue(const DestructorQueue &other) = delete;
   ~DestructorQueue();

   void
   Add(std::function<void(void)> &fn, error *err);

   void
   Destroy();
};

} // end namespace

#endif