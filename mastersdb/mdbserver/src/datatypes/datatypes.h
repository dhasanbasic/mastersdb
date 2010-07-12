/*
 * datatypes.h
 *
 * Data types supported by the database
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
 * 05.04.2010
 *    Initial version of file.
 * 09.04.2010
 *    Added a function for initializing the data type table.
 * 23.04.2010
 *    Added a function for data-type information retrieval.
 *    Added prototypes for special comparison functions.
 * 26.06.2010
 *    Re-factoring of data-type information retrieval function.
 */

#ifndef DATATYPES_H_
#define DATATYPES_H_

#include "../common.h"

/* forward declarations */
typedef struct mdbDatatype mdbDatatype;

#define MDB_TYPE_COUNT  5

struct mdbDatatype {
  byte name[6];           /* upper-case name, including null char.          */
  byte length;            /* length of the name                             */
  byte header;            /* length of header information (0 if not used)   */
  byte size;              /* size of the value, 0 for varying-size types    */
  CompareKeysPtr compare; /* pointer to comparison function                 */
};

/* special type comparison functions */
int mdbCompareFloat(const void* v1, const void* v2, uint32 size);
int mdbCompareUnicode(const void* v1, const void* v2, uint32 size);

/* initializes the Masters DB data type structures */
void mdbInitializeTypes(mdbDatatype **typetable);

/* retrieves a data type's information based on the name provided */
mdbDatatype* mdbGetTypeInfo(const byte *name, const byte length,
    mdbDatatype *typetable);

#endif /* DATATYPES_H_ */
