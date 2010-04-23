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
 */

#include "datatypes.h"

/* TODO: implement the special comparison functions */

int mdbCompareFloat(const void* v1, const void* v2, uint32 size)
{
  return 0;
}

int mdbCompareDouble(const void* v1, const void* v2, uint32 size)
{
  return 0;
}

int mdbCompareUnicode(const void* v1, const void* v2, uint32 size)
{
  return 0;
}

int mdbCompareLongDouble(const void* v1, const void* v2, uint32 size)
{
  return 0;
}


void mdbInitializeTypes(mdbDatatype **typetable)
{
  /* initialize the data types array */
  *typetable = (mdbDatatype*)malloc(MDB_TYPE_COUNT * sizeof(mdbDatatype));

  (*typetable)[0] = (mdbDatatype){
    "FLAG",          4, 0, sizeof(byte),        &memcmp};

  (*typetable)[1] = (mdbDatatype){
    "FLOAT",         5, 0, sizeof(float),       &mdbCompareFloat};

  (*typetable)[2] = (mdbDatatype){
    "DOUBLE",        6, 0, sizeof(double),      &mdbCompareDouble};

  (*typetable)[3] = (mdbDatatype){
    "BINARY",        6, 4, sizeof(byte),        NULL};

  (*typetable)[4] = (mdbDatatype){
    "INTEGER",       7, 0, sizeof(int),         &memcmp};

  (*typetable)[5] = (mdbDatatype){
    "ISOSTRING",     9, 4, sizeof(byte),        (CompareKeysPtr)&strncmp};

  (*typetable)[6] = (mdbDatatype){
    "UNISTRING",     9, 4, sizeof(byte) * 2,    &mdbCompareUnicode};

  (*typetable)[7] = (mdbDatatype){
    "UTF8STRING",   10, 4, sizeof(byte),        (CompareKeysPtr)&strncmp};

  (*typetable)[8] = (mdbDatatype){
    "LONG DOUBLE",  11, 0, sizeof(long double), &mdbCompareLongDouble};

  (*typetable)[9] = (mdbDatatype){
    "LONG INTEGER", 12, 0, sizeof(long int),    &memcmp};

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
