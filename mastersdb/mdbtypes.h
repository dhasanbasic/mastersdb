/*
 * mdbtypes.h
 *
 * Contains the common data structures
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
 * 10.08.2010
 *  Initial version of file.
 */

#ifndef MDBTYPES_H_
#define MDBTYPES_H_

/* Key/Data type comparison function */
typedef int (*CompareKeysPtr)(const void* key1, const void* key2, uint32 size);

/* B-tree node retrieval function */
typedef mdbBtreeNode* (*BtreeLoadNodePtr)
    (const uint32 position, mdbBtree* tree);

/* B-tree node write-out function */
typedef uint32 (*BtreeWriteNodePtr)(mdbBtreeNode* node);

/* B-tree node deletion function */
typedef void (*BtreeDeleteNodePtr)(mdbBtreeNode* node);

/* B-tree node meta-data */
struct mdbBtreeMeta
{
  uint32 record_size;     /* size of a record                       */
  uint32 key_position;    /* position of the primary key            */
  uint32 root_position;   /* position of the root node in the file  */
  uint32 order;           /* B-tree order (minimal children count)  */
};

/* B-tree structure */
struct mdbBtree
{
  mdbBtreeMeta meta;              /* node meta-data                        */
  uint32 nodeSize;                /* size of a node                        */
  mdbBtreeNode* root;             /* pointer to root node (preloaded)      */
  const mdbDatatype *key_type;    /* Data type of the B-tree key           */
  BtreeLoadNodePtr ReadNode;      /* node retrieval implementation         */
  BtreeWriteNodePtr WriteNode;    /* node write-out implementation         */
  BtreeDeleteNodePtr DeleteNode;  /* node deletion implementation          */
};

/* B-tree node structure */
struct mdbBtreeNode
{
  mdbBtree* T;            /* pointer to the B-tree                  */
  char *data;             /* raw data of the node                   */
  uint32 *record_count;   /* pointer to count of records            */
  uint32 *is_leaf;        /* pointer to leaf/internal information   */
  uint32 *children;       /* pointer to child pointer array         */
  char *records;          /* pointer to records                     */
  uint32 position;        /* position of node (data file)           */
};

/* B-tree traversal structure */
struct mdbBtreeTraversal
{
  mdbBtreeNode* node;         /* current B-tree node              */
  mdbBtreeTraversal* parent;  /* parent B-tree node (linked list) */
  uint32 position;            /* current record position          */
};

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

/* MastersDB data type */
struct mdbDatatype {
  char name[8];           /* upper-case name, including null char.          */
  byte header;            /* length of header information (0 if not used)   */
  byte size;              /* size of the value, 0 for varying-size types    */
  CompareKeysPtr compare; /* pointer to comparison function                 */
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

#endif /* COMMON_H_ */
