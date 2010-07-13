/*
 * libloader.h
 *
 * A simple Win32/Linux dynamic library loader implementation
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
 * 01.03.2010
 *      Initial version of file.
 */

#ifndef _BC_CLASS_BCLOADLIB_
#define _BC_CLASS_BCLOADLIB_

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

typedef struct
{
  void* libhandle;
} Library;

Library* OpenLibrary(const char* name);
void CloseLibrary(Library* lib);
void *GetFunction(const Library* lib, const char* name);

#endif // #ifndef _BC_CLASS_BCLOADLIB_

