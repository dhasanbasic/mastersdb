/*
 * datatypes.c
 *
 * Implementation of Masters DB datatypes and their comparison functions
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
 * 09.04.2010
 *    Initial version of file.
 * 23.04.2010
 *    Implemented the data type table initialization and data type retrieval.
 * 26.06.2010
 *    Re-factoring of data type table initialization.
 */

#include "datatypes.h"

/* TODO: implement the special comparison functions */

int mdbCompareFloat(const void* v1, const void* v2, uint32 size)
{
  return 0;
}

int mdbCompareUnicode(const void* v1, const void* v2, uint32 size)
{
  return 0;
}

void mdbInitializeTypes(mdbDatatype **typetable)
{
  /* initialize the data types array */
  *typetable = (mdbDatatype*)malloc(MDB_TYPE_COUNT * sizeof(mdbDatatype));

  (*typetable)[0] = (mdbDatatype){ "INT-16",  6, 0, sizeof(uint16), &memcmp};
  (*typetable)[1] = (mdbDatatype){ "INT-32",  6, 0, sizeof(uint32), &memcmp};
  (*typetable)[2] = (mdbDatatype){ "FLOAT",   5, 0, sizeof(float),
    &mdbCompareFloat};
  (*typetable)[3] = (mdbDatatype){ "CHAR-8",  6, 4, sizeof(byte),
    (CompareKeysPtr)&strncmp};
  (*typetable)[4] = (mdbDatatype){ "CHAR-16", 7, 4, sizeof(byte) * 2,
    &mdbCompareUnicode};
}

mdbDatatype* mdbGetTypeInfo(const byte *name, const byte length,
    mdbDatatype *typetable)
{
  byte i = 0;

  while (i < MDB_TYPE_COUNT) {
    /* if name length equals from given length, check if the names are equal */
    if (length == typetable[i].length) {
      if (!strncmp(name,typetable[i].name,length)) {
        return &typetable[i];
      }
    }
    i++;
  }
  return NULL;
}
