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
 *    Initial version of file.
 */

#ifndef COMMON_H_
#define COMMON_H_

/* unsigned integer types */
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned long ulong;
typedef unsigned char byte;

#include <string.h>
#include <malloc.h>


/* Key/Data type comparison function */
typedef int (*CompareKeysPtr)(const void* key1, const void* key2, uint32 size);

#endif /* COMMON_H_ */
