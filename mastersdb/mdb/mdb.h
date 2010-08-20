
/*
 * mdb.h
 *
 * MastersDB main include file.
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
 * 20.07.2010
 *  Renamed project to MastersDB. Server functionality will not be included.
*/

#ifndef MDB_H_INCLUDED
#define MDB_H_INCLUDED

#include "stdint.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"

/* MastersDB format signature (magic number) */
#define MDB_MAGIC_NUMBER  0xEEDB

/* MastersDB version signature (0.8) */
#define MDB_VERSION       0x0008

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
typedef struct mdbTableRecord mdbTableRecord;
typedef struct mdbColumnRecord mdbColumnRecord;
typedef struct mdbIndexRecord mdbIndexRecord;

#include "mdbtypes.h"

/* ********************************************************* */
/* ********************************************************* */

/* ********************************************************* *
 *    B-tree related functions
 * ********************************************************* */
/* B-tree allocation and initialization */
int mdbBtreeCreate(mdbBtree** tree,
    uint32 order,
    const uint32 record_size,
    const uint32 key_position);

/* B-tree node allocation function */
int mdbAllocateNode(mdbBtreeNode** node, mdbBtree *tree);

/* B-tree search */
int mdbBtreeSearch(const char* key, char* record, mdbBtree* t);

/* B-tree insertion */
int mdbBtreeInsert(const char* record, mdbBtree* t);

/* B-tree deletion */
int mdbBtreeDelete(const char* key, mdbBtree* t);

/* B-tree traversal */
int mdbBtreeTraverse(mdbBtreeTraversal **t, char *record);

/* General return values */
#define MDB_BTREE_SUCCESS          1  /* B-tree operation succeeded         */
#define MDB_BTREE_NOROOT           0  /* B-tree contains no root pointer    */
#define MDB_BTREE_NOTFOUND        -1  /* non-existing key                   */

/* B-tree search function return values */
#define MDB_BTREE_SEARCH_FOUND      MDB_BTREE_SUCCESS
#define MDB_BTREE_SEARCH_NOTFOUND   MDB_BTREE_NOTFOUND

/* B-tree insertion function return values */
#define MDB_BTREE_INSERT_COLLISION  -2  /* tree already contains that key   */
#define MDB_BTREE_INSERT_NOROOT     MDB_BTREE_NOROOT
#define MDB_BTREE_INSERT_SUCCESS    MDB_BTREE_SUCCESS

/* B-tree deletion function return values */
#define MDB_BTREE_DELETE_NOTFOUND   MDB_BTREE_NOTFOUND
#define MDB_BTREE_DELETE_NOROOT     MDB_BTREE_NOROOT
#define MDB_BTREE_DELETE_SUCCESS    MDB_BTREE_SUCCESS
#define MDB_BTREE_DELETE_EMPTYROOT  2  /* B-tree root node has no records   */

/* ********************************************************* */
/* ********************************************************* */

/* ********************************************************* *
 *    Database related functions and defines
 * ********************************************************* */
/* Creates an empty MastersDB database */
int mdbCreateDatabase(mdbDatabase **db, const char *filename);

/* Loads an existing MastersDB database, including header check */
int mdbOpenDatabase(mdbDatabase **db, const char *filename);

/* Loads an existing MastersDB database, including header check */
int mdbCloseDatabase(mdbDatabase *db);

/* General return values */
#define MDB_DATABASE_SUCCESS          1  /* Database creation succeeded      */
#define MDB_DATABASE_NOFILE          -1  /* Database creation failure (I/O)  */
#define MDB_DATABASE_INVALIDFILE     -2  /* Tried to open invalid file       */
#define MDB_DATABASE_NO_SUCH_TABLE   -3  /* Tried to load non-existing table */

/* Data-type count */
#define MDB_DATATYPE_COUNT  5

/* ********************************************************* */
/* ********************************************************* */

/* ********************************************************* *
 *    Table related functions
 * ********************************************************* */
/* Loads the meta data, B-tree descriptor and root node of a table */
int mdbFreeTable(mdbTable *t);

/* creates the system tables and writes them to a file */
int mdbCreateSystemTables(mdbDatabase *db);

/* Loads meta data, B-tree descriptor and root node of the system tables */
int mdbLoadSystemTables(mdbDatabase *db);

/* Creates a table and stores its B-tree and root node into the database */
int mdbCreateTable(mdbTable *t);

/* Loads the meta data, B-tree descriptor and root node of a table */
int mdbLoadTable(mdbDatabase *db, mdbTable **t, const char *name);

/* General return values */
#define MDB_TABLE_SUCCESS             1  /* Table creation succeeded  */
#define MDB_TABLE_NOTFOUND           -1  /* Table does not exist      */

/* ********************************************************* */
/* ********************************************************* */


#endif
