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
 */

#ifndef DATABASE_H_INCLUDED
#define DATABASE_H_INCLUDED

#include "common.h"
#include "btree.h"

/* forward declarations of the database structures */
typedef struct mdbFreeEntry mdbFreeEntry;
typedef struct mdbDatabaseMeta mdbDatabaseMeta;
typedef struct mdbDatabase mdbDatabase;
typedef struct mdbTable mdbTable;
typedef struct mdbField mdbField;
typedef struct mdbIndex mdbIndex;

/* Creates an empty MastersDB database */
int mdbCreateDatabase(mdbDatabase **db, const char *filename);

/* Loads an existing MastersDB database, including header check */
int mdbOpenDatabase(mdbDatabase **db, const char *filename);

/* Loads an existing MastersDB database, including header check */
int mdbCloseDatabase(const mdbDatabase *db);

/* General return values */
#define MDB_DATABASE_SUCCESS          1  /* Database creation succeeded      */
#define MDB_DATABASE_NOFILE          -1  /* Database creation failure (I/O)  */
#define MDB_DATABASE_INVALIDFILE     -2  /* Tried to open invalid file       */

/* System tables B-tree parameters */
#define MDB_TABLES_ORDER          964
#define MDB_TABLES_KEY_SIZE       59

#define MDB_FIELDS_ORDER          7944
#define MDB_FIELDS_KEY_SIZE       64

#define MDB_INDEXES_ORDER         7282
#define MDB_INDEXES_KEY_SIZE      64

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
  uint32 sys_tables[3];         /* System tables B-tree positions   */
  uint32 usr_tables[20];        /* User tables B-tree positions     */
  mdbFreeEntry free_space[20];  /* Free blocks table                */
};

/* MastersDB database runtime information */
struct mdbDatabase
{
  mdbDatabaseMeta meta;
  mdbBtree *tables;
  mdbBtree *fields;
  mdbBtree *indexes;
  FILE *file;
};

/* MastersDB table record */
struct mdbTable
{
  uint32 name_header;           /* Length of table name             */
  char name[55];                /* Table name                       */
  byte num_fields;              /* Number of fields                 */
  uint32 btree;                 /* Pointer to B-tree in the file    */
};

/* MastersDB field record */
struct mdbField
{
  uint32 id_header;             /* Length of field identifier       */
  char id[60];                  /* Field identifier (table_name + N)*/
  uint32 type_header;           /* Length of data type name         */
  char type[8];                 /* Data type name                   */
  uint32 name_header;           /* Length of field name             */
  char name[43];                /* field name                       */
  byte indexed;                 /* >0 = The field is indexed        */
  uint32 length;                /* max. length of the field value   */
};

/* MastersDB index record */
struct mdbIndex
{
  uint32 id_header;             /* Length of index identifier       */
  char id[60];                  /* Index identifier (field id)      */
  uint32 btree;                 /* Pointer to B+-tree in the file   */
};

#endif
