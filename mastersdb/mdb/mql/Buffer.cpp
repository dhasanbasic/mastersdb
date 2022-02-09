/*
 * Buffer.cpp
 *
 * A stripped down version of the Buffer and Buffer classes from Coco/R
 * C++ joined together, supporting only in-memory character streams.
 *
 * Copyright (C) 2010, Dinko Hasanbasic (dinko.hasanbasic@gmail.com)
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

#include "Buffer.h"

namespace MDB
{

Buffer::Buffer(const unsigned char* buf, int len) {
  this->buf = new unsigned char[len];
  memcpy(this->buf, buf, len*sizeof(unsigned char));
  bufStart = 0;
  bufCapacity = bufLen = len;
  fileLen = len;
  bufPos = 0;
}

Buffer::Buffer(Buffer *b) {
  buf = b->buf;
  bufCapacity = b->bufCapacity;
  b->buf = NULL;
  bufStart = b->bufStart;
  bufLen = b->bufLen;
  fileLen = b->fileLen;
  bufPos = b->bufPos;
}


Buffer::~Buffer() {
  if (buf != NULL) {
    delete [] buf;
    buf = NULL;
  }
}

int Buffer::_read() {
  if (bufPos < bufLen) {
    return buf[bufPos++];
  } else if (GetPos() < fileLen) {
    SetPos(GetPos()); // shift buffer start to Pos
    return buf[bufPos++];
  } else {
    return EoF;
  }
}

int Buffer::Peek() {
  int curPos = GetPos();
  int ch = Read();
  SetPos(curPos);
  return ch;
}

wchar_t* Buffer::GetString(int beg, int end) {
  int len = 0;
  wchar_t *buf = new wchar_t[end - beg];
  int oldPos = GetPos();
  SetPos(beg);
  while (GetPos() < end) buf[len++] = (wchar_t) Read();
  SetPos(oldPos);
  wchar_t *res = coco_string_create(buf, 0, len);
  coco_string_delete(buf);
  return res;
}

int Buffer::GetPos() {
  return bufPos + bufStart;
}

void Buffer::SetPos(int value) {

  if ((value < 0) || (value > fileLen)) {
    wprintf(L"--- buffer out of bounds access, position: %d\n", value);
    exit(1);
  }

  if ((value >= bufStart) && (value < (bufStart + bufLen))) { // already in buffer
    bufPos = value - bufStart;
  } else {
    bufPos = fileLen - bufStart; // make Pos return fileLen
  }
}

int Buffer::Read() {
  int ch;
  do {
    ch = _read();
    // until we find a utf8 start (0xxxxxxx or 11xxxxxx)
  } while ((ch >= 128) && ((ch & 0xC0) != 0xC0) && (ch != EoF));
  if (ch < 128 || ch == EoF) {
    // nothing to do, first 127 chars are the same in ascii and utf8
    // 0xxxxxxx or end of file character
  } else if ((ch & 0xF0) == 0xF0) {
    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    int c1 = ch & 0x07; ch = _read();
    int c2 = ch & 0x3F; ch = _read();
    int c3 = ch & 0x3F; ch = _read();
    int c4 = ch & 0x3F;
    ch = (((((c1 << 6) | c2) << 6) | c3) << 6) | c4;
  } else if ((ch & 0xE0) == 0xE0) {
    // 1110xxxx 10xxxxxx 10xxxxxx
    int c1 = ch & 0x0F; ch = _read();
    int c2 = ch & 0x3F; ch = _read();
    int c3 = ch & 0x3F;
    ch = (((c1 << 6) | c2) << 6) | c3;
  } else if ((ch & 0xC0) == 0xC0) {
    // 110xxxxx 10xxxxxx
    int c1 = ch & 0x1F; ch = _read();
    int c2 = ch & 0x3F;
    ch = (c1 << 6) | c2;
  }
  return ch;
}

}
