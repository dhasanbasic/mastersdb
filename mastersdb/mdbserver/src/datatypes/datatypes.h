/*
 * datatypes.h
 *
 * Datatypes supported by the database
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
 */

#ifndef DATATYPES_H_
#define DATATYPES_H_

#include "../common.h"

/* forward declarations */
typedef struct MdbDatatype MdbDatatype;

struct MdbDatatype {
  uint16 id;              /* index of the data type in the type-names table   */
  uint16 nameLength;      /* length of the type-name                          */
  uint16 size;            /* size of the data type, 0 for varying-size types  */
  byte name[16];          /* name of the type, used in SQL                    */
  CompareKeysPtr compare; /* pointer to comparison function                   */
};




#endif /* DATATYPES_H_ */
