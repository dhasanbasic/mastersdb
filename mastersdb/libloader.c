
/*
 * libloader.c
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
 *    Initial version of file.
 */

#include "libloader.h"
#include <malloc.h>
#include <string.h>

Library* OpenLibrary(const char* name) {

  Library* lib = (Library*)malloc(sizeof(Library));
  unsigned int len;
  char* libname;

  len = 0;
  libname = NULL;
	lib->libhandle = NULL;

  #ifdef WIN32
    lib->libhandle = LoadLibrary(name);
	#else
    /* adds "./" at the begginning, and ".so\0" at the end */
    len = strlen(name);
    len += 6;
    libname = (char*)malloc(len);
    libname[0] = 0;
    strcat(libname, "./");
    strcat(libname, libname);
    strcat(libname, ".so");
    lin->libhandle = dlopen(libname, RTLD_LAZY);
    free(libname);
	#endif

	if(!lib->libhandle) {
    free(lib);
    return NULL;
  }
  return lib;
}

void CloseLibrary(Library* lib) {
	if(lib->libhandle){
		#ifdef WIN32
		FreeLibrary((HINSTANCE)lib->libhandle);
		#else
		dlclose(lib->libhandle);
		#endif
	}
  free(lib);
}

void *GetFunction(const Library* lib, const char* name) {
	void* func = 0;
	if(lib->libhandle){
		#ifdef WIN32
		func = GetProcAddress((HINSTANCE)lib->libhandle, name);
		#else
		func = dlsym(lib->libhandle, name);
		#endif
	}
	if (!func)
    return NULL;
  return func;
}
