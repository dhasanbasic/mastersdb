
/*
 * mdb.h
 *
 * MastersDB main include file.
 *
 * Copyright (C) 2010, Dinko Hasanbasic (dinko.hasanbasic@gmail.com)
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
 * 20.07.2010
 *  Renamed project to MastersDB. Server functionality will not be included.
 * 02.09.2010
 *  Moved all function return values and error codes to a new enumerator.
 *  Removed the mdbTable structure. It will be handled by the virtual machine.
*/

#ifndef MDB_H_
#define MDB_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef __clang__
	#include <malloc.h>
#else
	#include <stdlib.h>
#endif

/* MastersDB format signature (magic number) */
#define MDB_MAGIC_NUMBER  0xEEDB

/* MastersDB version signature (0.8) */
#define MDB_VERSION       0x0008

/* MastersDB error codes enumerator*/
typedef enum mdbError
{
  MDB_NO_ERROR = 0,
  MDB_BTREE_NO_ROOT,
  MDB_BTREE_KEY_NOT_FOUND,
  MDB_BTREE_KEY_COLLISION,
  MDB_BTREE_ROOT_IS_EMPTY,
  MDB_BTREE_NO_MORE_RECORDS,
  MDB_CANNOT_CREATE_FILE,
  MDB_INVALID_FILE,
  MDB_TABLE_NOT_FOUND
}  mdbError;

/* ********************************************************* *
 *    Common type definitions and data structures
 * ********************************************************* */
/* unsigned integer types */
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t  uint8;
typedef uint8_t byte;

/* forward declarations of the B-tree structures */
typedef struct mdbBtreeMeta       mdbBtreeMeta;
typedef struct mdbBtree           mdbBtree;
typedef struct mdbBtreeNode       mdbBtreeNode;
typedef struct mdbBtreeTraversal  mdbBtreeTraversal;
typedef struct mdbDatatype        mdbDatatype;

/* forward declarations of the database structures */
typedef struct mdbFreeEntry mdbFreeEntry;
typedef struct mdbDatabaseMeta mdbDatabaseMeta;
typedef struct mdbDatabase mdbDatabase;

/* forward declarations of the table structures */
typedef struct mdbTable mdbTable;
typedef struct mdbColumn mdbColumn;
typedef struct mdbIndex mdbIndex;

#include "mdbtypes.h"

/* ********************************************************* */
/* ********************************************************* */

/* ********************************************************* *
 *    B-tree related functions
 * ********************************************************* */
/* B-tree allocation and initialization */
mdbError mdbBtreeCreate(mdbBtree** tree,
    uint32 order,
    const uint32 record_size,
    const uint32 key_position);

/* B-tree node allocation function */
mdbError mdbAllocateNode(mdbBtreeNode** node, mdbBtree *tree);

/* Frees up the memory used by a B-tree node (with eventual save) */
mdbError mdbFreeNode(mdbBtreeNode* node, uint8 save);

/* B-tree search */
mdbError mdbBtreeSearch(const char* key, char* record, mdbBtree* t);

/* B-tree insertion */
mdbError mdbBtreeInsert(const char* record, mdbBtree* t);

/* B-tree deletion */
mdbError mdbBtreeDelete(const char* key, mdbBtree* t);

/* B-tree traversal */
mdbError mdbBtreeTraverse(mdbBtreeTraversal **t, char *record);

/* ********************************************************* */
/* ********************************************************* */

/* ********************************************************* *
 *    Database related functions and defines
 * ********************************************************* */
/* Creates an empty MastersDB database */
mdbError mdbCreateDatabase(mdbDatabase **db, const char *filename);

/* Loads an existing MastersDB database, including header check */
mdbError mdbOpenDatabase(mdbDatabase **db, const char *filename);

/* Loads an existing MastersDB database, including header check */
mdbError mdbCloseDatabase(mdbDatabase *db);

/* Data-type count */
#define MDB_DATATYPE_COUNT  5

/* ********************************************************* */
/* ********************************************************* */

/* ********************************************************* *
 *    Table related functions
 * ********************************************************* */

/* creates the system tables and writes them to a file */
mdbError mdbCreateSystemTables(mdbDatabase *db);

/* Creates a table and stores its B-tree and root node in the database */
mdbError mdbCreateTable(
    mdbDatabase *db,
    const char *name,
    uint8 num_columns,
    uint32 record_size,
    mdbBtree** btree,
    void *cls,
    mdbColumnRetrievalPtr cb);

/* Loads the meta data, B-tree descriptor and root node of a table */
mdbError mdbLoadTable(
    mdbDatabase *db,
    const char *name,
    mdbBtree **btree,
    void *cls,
    mdbColumnCallbackPtr cb);

/* ********************************************************* */
/* ********************************************************* */


#endif /* MDB_H_ */
