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
 */

#include "datatypes.h"

void MdbHashTypename(const byte *name, byte *hash, uint16 *checksum)
{
  short lHash = -170;
  byte l = (name[0] == 'L') ? 2 : 0;
  byte u = l + 4;

  *checksum = 0;
  while (++l < u) {
    lHash += name[l];
    *checksum += (u-l) * name[l];
  }

  lHash %= 38;
  lHash %= 28;
  lHash %= 12;
  *hash = (lHash > 10) ? 7 : lHash;
}

void MdbInitializeTypes(MdbDatatype **datatypes)
{
  byte index;
  uint16 checksum;

  const byte *mdbFlag = "FLAG";
  /*
  const byte *mdbFloat = "FLOAT";
  const byte *mdbDouble = "DOUBLE";
  const byte *mdbBinary = "BINARY";
  const byte *mdbInteger = "INTEGER";
  const byte *mdbCurrency = "CURRENCY";
  const byte *mdbISOString = "ISOSTRING";
  const byte *mdbUTF8String = "UTF8STRING";
  const byte *mdbUTF16String = "UTF16STRING";
  const byte *mdbLongDouble = "LONG DOUBLE";
  const byte *mdbLongInteger = "LONG INTEGER";
  */

  /* initialize the data types array */
  *datatypes = (MdbDatatype*)malloc(MDB_DATATYPE_COUNT * sizeof(MdbDatatype));

  /* initialize the FLAG data type */
  MdbHashTypename(mdbFlag, &index, &checksum);
  datatypes[index]->checksum = checksum;
  datatypes[index]->nameLength = strlen(mdbFlag);
  datatypes[index]->name = malloc(datatypes[index]->nameLength);
  strncpy(datatypes[index]->name, mdbFlag, datatypes[index]->nameLength);
  datatypes[index]->compare = &memcmp;
  datatypes[index]->size = 1;

  /* TODO: initialize the remaining data types */

}
