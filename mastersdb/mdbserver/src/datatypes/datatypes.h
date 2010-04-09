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
 *    Added a specific hashing function for the predefined type names.
 *    Added a function for initializing the data type table.
 */

#ifndef DATATYPES_H_
#define DATATYPES_H_

#include "../common.h"

/* forward declarations */
typedef struct MdbDatatype MdbDatatype;

#define MDB_DATATYPE_COUNT    11

struct MdbDatatype {
  uint16 checksum;        /* index of the data type in the type-names table   */
  uint16 nameLength;      /* length of the type-name                          */
  uint16 size;            /* size of the data type, 0 for varying-size types  */
  byte *name;             /* name of the type, used in SQL                    */
  CompareKeysPtr compare; /* pointer to comparison function                   */
};

/* initializes the Masters DB data type structures */
void MdbInitializeTypes(MdbDatatype **datatypes);

/*
 * generates a index between 0 and 10, and a unique checksum for a given
 * Masters DB data type name
 */
void MdbHashTypename(const byte *name, byte *hash, uint16 *checksum);

#endif /* DATATYPES_H_ */
