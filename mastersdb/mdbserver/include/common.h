/*
 * common.h
 *
 * ### DESCRIPTION
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
 * 31.03.2010
 *  Initial version of file.
 * 13.07.2010
 *  Added the MasterDB version and format magic number definitions
 * 15.07.2010
 *  Switched the main data types to stdint.h (platform independent).
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <string.h>
#include <malloc.h>
#include <stdint.h>

/* unsigned integer types */
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t  uint8;
typedef uint8_t byte;

/* MastersDB format signature (magic number) */
#define MDB_MAGIC_NUMBER  0xEEDB

/* MastersDB version signature (0.7) */
#define MDB_VERSION       0x0007

/* Key/Data type comparison function */
typedef int (*CompareKeysPtr)(const void* key1, const void* key2, uint32 size);

#endif /* COMMON_H_ */
