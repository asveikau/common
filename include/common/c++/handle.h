/*
 Copyright (C) 2019 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_cxx_handle_h_
#define common_cxx_handle_h_

#if defined(_WINDOWS)
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace common {

template <typename Info>
class Handle
{
   typename Info::T value;

public:

   Handle() : value(Info::InitialValue) {}
   Handle(const Handle<Info> &other) = delete;
   Handle(typename Info::T value_) : value(value_) {}
   Handle(Handle<Info> &&other) : value(other.value)
   {
      other.value = Info::InitialValue;
   }
   ~Handle()
   {
      Reset();
   }

   void
   Reset()
   {
      if (Valid())
         Info::Close(value);
   }

   void
   Detach()
   {
      value = Info::InitialValue;
   }

   bool
   Valid()
   {
      return Info::Valid(value);
   }

   typename Info::T
   Get()
   {
      return value;
   }

   Handle<Info> &
   operator = (typename Info::T value)
   {
      Reset();
      this->value = value;
      return *this;
   }

   Handle<Info> &
   operator = (Handle<Info> &&other)
   {
      Reset();
      value = other.value;
      other.value = Info::InitialValue;
      return *this;
   }
};

#if defined(_WINDOWS)
namespace internal {
struct FileHandleInfo
{
   typedef HANDLE T;
   static constexpr HANDLE InitialValue = INVALID_HANDLE_VALUE;
   static bool Valid(HANDLE h) { return h && (h != INVALID_HANDLE_VALUE); }
   static void Close(HANDLE h) { CloseHandle(h); }
};
struct SocketHandleInfo
{
   typedef SOCKET T;
   static const SOCKET InitialValue = INVALID_SOCKET;
   static bool Valid(SOCKET sock) { return sock != INVALID_SOCKET; }
   static void Close(SOCKET sock) { closesocket(sock); }
};
} // end namespace
typedef Handle<internal::FileHandleInfo> FileHandle;
typedef Handle<internal::SocketHandleInfo> SocketHandle;
#else
namespace internal {
struct FileHandleInfo
{
   typedef int T;
   static const int InitialValue = -1;
   static bool Valid(int fd) { return fd >= 0; }
   static void Close(int fd) { close(fd); }
};
} // end namespace
typedef Handle<internal::FileHandleInfo> FileHandle;
typedef FileHandle SocketHandle;
#endif

} // end namespace

#endif
