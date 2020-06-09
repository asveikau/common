/*
 Copyright (C) 2020 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#ifndef common_cpp_linereader_h
#define common_cpp_linereader_h

#include "stream.h"

#include <vector>

namespace common {

class LineReader
{
   common::Pointer<common::Stream> stream;
   char buf[1024];
   size_t off;
   size_t inBuf;
   std::vector<unsigned char> lineBuf;
public:
   LineReader(common::Stream *str);
   LineReader(const LineReader &) = delete;

   int
   ReadChar(error *err);

   char *
   ReadLine(error *err);
};

} // end namespace

#endif