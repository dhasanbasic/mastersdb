/*
 * Buffer.h
 *
 * A stripped down version of the Buffer and Buffer classes from Coco/R
 * C++ joined together, supporting only in-memory character streams.
 *
 * Copyright (C) 2010, Dinko Hasanbasic (dinkoh@bih.net.ba)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Revision history
 * ----------------
 * 20.08.2010
 *  Initial version of file.
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <cstring>
#include <cstdlib>
#include <wchar.h>

// io.h and fcntl are used to ensure binary read from streams on windows
#if _MSC_VER >= 1300
#include <io.h>
#include <fcntl.h>
#endif

#if _MSC_VER >= 1400
#define coco_swprintf swprintf_s
#elif _MSC_VER >= 1300
#define coco_swprintf _snwprintf
#else
// assume every other compiler knows swprintf
#define coco_swprintf swprintf
#endif

#define COCO_WCHAR_MAX 65535

extern wchar_t* coco_string_create(const wchar_t *value, int startIndex, int length);
extern void  coco_string_delete(wchar_t* &data);


namespace MDB
{

class Buffer {
private:

  unsigned char *buf; // input buffer
  int bufCapacity;    // capacity of buf
  int bufStart;       // position of first byte in buffer relative to input stream
  int bufLen;         // length of buffer
  int fileLen;        // length of input stream (may change if the stream is no file)
  int bufPos;         // current position in buffer

  int _read();

public:
  static const int EoF = COCO_WCHAR_MAX + 1;

  Buffer(const unsigned char* buf, int len);
  Buffer(Buffer *b);
  virtual ~Buffer();
  virtual int Read();
  virtual int Peek();
  virtual wchar_t* GetString(int beg, int end);
  virtual int GetPos();
  virtual void SetPos(int value);
};

}

#endif /* BUFFER_H_ */
