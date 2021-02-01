/*
 Copyright (C) 2020 Andrew Sveikauskas

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
*/

#include <common/c++/linereader.h>

common::LineReader::LineReader(common::Stream *str) : stream(str), off(0), inBuf(0) {}

int
common::LineReader::ReadChar(error *err)
{
   int c = EOF;

   if (off == inBuf)
   {
      off = inBuf = 0;

      inBuf = stream->Read(buf, sizeof(buf), err);
      ERROR_CHECK(err);
      if (!inBuf)
         goto exit;
   }

   c = buf[off++];
exit:
   return c;
}

char *
common::LineReader::ReadLine(error *err)
{
   // TODO: if we can find a line terminator within the current buffer,
   // return it directly.

   lineBuf.resize(0);

   for (;;)
   {
      int c = ReadChar(err);
      ERROR_CHECK(err);

      switch (c)
      {
      case EOF:
         if (!lineBuf.size())
            goto exit;
         c = 0;
         break;
      case '\r':
         // Try to convert CRLF to LF.
         c = ReadChar(err);
         ERROR_CHECK(err);
         // Put back any non-'\n' char
         if (c != EOF && c != '\n')
            off--;
         // fall through
      case '\n':
         c = 0;
         break;
      }

      try
      {
         lineBuf.push_back(c);
      }
      catch (const std::bad_alloc&)
      {
         ERROR_SET(err, nomem);
      }
      if (c == 0)
         break;
   }

   return (char*)lineBuf.data();
exit:
   return nullptr;
}