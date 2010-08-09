/*
 * database.h
 *
 * Database creation and read function prototypes
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
 * 13.07.2010
 *  Initial version of file.
 * 15.07.2010
 *  Added system table constants and structures for tables, fields
 *  and indexes.
 * 19.07.2010
 *  Reordered fields in the data structures to avoid data alignment.
 * 20.07.2010
 *  Added prototypes for the remaining database functions.
 * 25.07.2010
 *  Moved data-type related code to here.
 * 07.08.2010
 *  Moved mdbDatatype structure to btree.h.
 * 08.08.2010
 *  Restructured the database.
 *  Added mdbCreateTable function.
 *  Added mdbLoadSystemTables function.
 * 09.08.2010
 *  Added mdbLoadTable function.
 *  Added mdbFreeTable function.
 *  Made mdbAllocateTable publicly visible.
 *  Implemented mdbCreateTable function.
 */

#ifndef DATABASE_H_INCLUDED
#define DATABASE_H_INCLUDED

#include "../common.h"
#include "../btree/btree.h"

/* forward declarations of the database structures */
typedef struct mdbFreeEntry mdbFreeEntry;
typedef struct mdbDatabaseMeta mdbDatabaseMeta;
typedef struct mdbDatabase mdbDatabase;
typedef struct mdbTable mdbTable;
typedef struct mdbColumn mdbColumn;
typedef struct mdbIndex mdbIndex;

/* Creates an empty MastersDB database */
int mdbCreateDatabase(mdbDatabase **db, const char *filename);

/* Loads an existing MastersDB database, including header check */
int mdbOpenDatabase(mdbDatabase **db, const char *filename);

/* Loads an existing MastersDB database, including header check */
int mdbCloseDatabase(mdbDatabase *db);

/* Allocates a new mdbTable structure and initializes the internal pointers */
int mdbAllocateTable(mdbTable **table);

/* Loads the meta data, B-tree descriptor and root node of a table */
int mdbFreeTable(mdbTable *t);

/* Creates a table and stores its B-tree and root node into the database */
int mdbCreateTable(mdbTable *t);

/* Loads the meta data, B-tree descriptor and root node of a table */
int mdbLoadTable(mdbDatabase *db, mdbTable **t, const char *name);

/* General return values */
#define MDB_DATABASE_SUCCESS          1  /* Database creation succeeded      */
#define MDB_DATABASE_NOFILE          -1  /* Database creation failure (I/O)  */
#define MDB_DATABASE_INVALIDFILE     -2  /* Tried to open invalid file       */
#define MDB_DATABASE_NO_SUCH_TABLE   -3  /* Tried to load non-existing table */

/* System tables B-tree parameters */
#define MDB_TABLES_ORDER          964
#define MDB_COLUMNS_ORDER         7944
#define MDB_INDEXES_ORDER         7282

/* Data-type count */
#define MDB_TYPE_COUNT  5

/* MastersDB free entry (element of free entry table) */
struct mdbFreeEntry
{
  uint32 size;                  /* Size of free block (in bytes)    */
  uint32 position;              /* Position of free block in file   */
};

/* MastersDB database meta data */
struct mdbDatabaseMeta
{
  uint16 magic_number;          /* MastersDB format magic number    */
  uint16 mdb_version;           /* MastersDB version                */
  mdbFreeEntry free_space[16];  /* Free blocks table                */
};

/* MastersDB database runtime information */
struct mdbDatabase
{
  mdbDatabaseMeta meta;
  mdbTable *tables;
  mdbTable *columns;
  mdbTable *indexes;
  mdbDatatype *datatypes;
  FILE *file;
};

/* MastersDB table record */
struct mdbTable
{
  char data[64];                /* the table record                 */
  char *name;                   /* Table name                       */
  byte *num_columns;            /* Number of fields                 */
  uint32* position;             /* Pointer to B-tree in the file    */
  mdbColumn *columns;           /* Field information                */
  mdbBtree* T;                  /* Table's B-tree                   */
  mdbDatabase *db;              /* Pointer to database              */
};

/* MastersDB field record */
struct mdbColumn
{
  char id[64];                  /* Field identifier (table_name + N)*/
  char name[58];                /* Data type name                   */
  byte type;                    /* field name                       */
  byte indexed;                 /* >0 = The field is indexed        */
  uint32 length;                /* max. length of the field value   */
};

/* MastersDB index record */
struct mdbIndex
{
  char id[64];                  /* Index identifier (field id)      */
  uint32 btree;                 /* Pointer to B+-tree in the file   */
};

#endif
