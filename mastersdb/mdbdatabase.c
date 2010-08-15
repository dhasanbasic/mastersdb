/*
 * mdbdatabase.c
 *
 * Implementation of Masters DB database-specific functions
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
 * 14.07.2010
 *  Implemented a utility function for the system tables creation.
 * 19.07.2010
 *  Finished implementation of system tables creation utility function.
 * 20.07.2010
 *  Implemented remaining database functions.
 * 25.07.2010
 *  Moved data-type related code to here.
 * 07.08.2010
 *  Fixed few bugs in the database initialization.
 * 08.08.2010
 *  Huge re-factoring of database structures and create/open logic.
 *  Added mdbCreateTable function.
 *  Added mdbLoadSystemTables function.
 * 09.08.2010
 *  Added mdbLoadTable function.
 *  Re-factoring of the mdbDatatype structure.
 *  Added mdbFreeTable function.
 *  Implemented mdbCreateTable function.
 * 10.08.2010
 *  Moved table specific code to new file: table.c.
 */

#include "mastersdb.h"

/* TODO: implement the special comparison functions */

int mdbCompareFloat(const void* v1, const void* v2, uint32 size)
{
  return 0;
}

void mdbInitializeTypes(mdbDatabase *db)
{
  /* initialize the data types array */
  db->datatypes = (mdbDatatype*)calloc(MDB_DATATYPE_COUNT, sizeof(mdbDatatype));

  db->datatypes[0] = (mdbDatatype){ "INT-8",  0, sizeof(byte),   &memcmp};

  db->datatypes[1] = (mdbDatatype){ "INT-16", 0, sizeof(uint16), &memcmp};

  db->datatypes[2] = (mdbDatatype){ "INT-32", 0, sizeof(uint32), &memcmp};

  db->datatypes[3] = (mdbDatatype){ "FLOAT" , 0, sizeof(float),
    &mdbCompareFloat};

  db->datatypes[4] = (mdbDatatype){ "STRING", 4, sizeof(byte),
    (CompareKeysPtr)&strncmp};
}

int mdbCreateDatabase(mdbDatabase **db, const char *filename)
{
  mdbDatabase *l_db = (mdbDatabase*)malloc(sizeof(mdbDatabase));
  mdbInitializeTypes(l_db);

  /* creates the MastersDB header */
  memset(&l_db->meta, 0, sizeof(mdbDatabaseMeta));
  l_db->meta.magic_number = MDB_MAGIC_NUMBER;
  l_db->meta.mdb_version = MDB_VERSION;

  /* writes the MastersDB header and meta-data to a file */
  if ((l_db->file = fopen(filename, "w+b")) != NULL)
  {
    mdbCreateSystemTables(l_db);

    /* positions of the database system tables (B-tree + root node) */
    l_db->tables->T->meta.root_position =
        l_db->indexes->rec.btree + sizeof(mdbBtreeMeta);

    l_db->columns->T->meta.root_position =
        l_db->tables->T->meta.root_position + l_db->tables->T->nodeSize;

    l_db->indexes->T->meta.root_position =
        l_db->columns->T->meta.root_position + l_db->columns->T->nodeSize;

    /* writes all meta data to the empty database */
    fwrite(&(l_db->meta), sizeof(mdbDatabaseMeta), 1, l_db->file);

    fwrite(&(l_db->tables->T->meta), sizeof(mdbBtreeMeta), 1, l_db->file);
    fwrite(&(l_db->columns->T->meta), sizeof(mdbBtreeMeta), 1, l_db->file);
    fwrite(&(l_db->indexes->T->meta), sizeof(mdbBtreeMeta), 1, l_db->file);

    fwrite(l_db->tables->T->root->data,
        l_db->tables->T->nodeSize, 1, l_db->file);
    fwrite(l_db->columns->T->root->data,
        l_db->columns->T->nodeSize,1, l_db->file);
    fwrite(l_db->indexes->T->root->data,
        l_db->indexes->T->nodeSize,1, l_db->file);
  }
  else
  {
    free(l_db);
    return MDB_DATABASE_NOFILE;
  }

  *db = l_db;
  return MDB_DATABASE_SUCCESS;
}

int mdbOpenDatabase(mdbDatabase **db, const char *filename)
{
  mdbDatabase *l_db = (mdbDatabase*)malloc(sizeof(mdbDatabase));
  uint32 size_test = sizeof(mdbDatabaseMeta) + 3 * sizeof(mdbBtreeMeta);
  mdbBtree *T;
  mdbBtreeMeta meta;
  int ret;

  mdbInitializeTypes(l_db);

  /* creates the MastersDB header */
  memset(&l_db->meta, 0, sizeof(mdbDatabaseMeta));

  /* reads the MastersDB header and meta-data from a file */
  if ((l_db->file = fopen(filename, "r+b")) != NULL)
  {
    /* checks the file size */
    if (fseek(l_db->file, size_test, SEEK_SET) != 0)
    {
      fclose(l_db->file);
      free(l_db);
      return MDB_DATABASE_INVALIDFILE;
    }

    /* reads the header and checks the magic number and version */
    fseek(l_db->file, 0L, SEEK_SET);
    fread(&l_db->meta, sizeof(mdbDatabaseMeta), 1, l_db->file);
    if (l_db->meta.magic_number != MDB_MAGIC_NUMBER ||
        l_db->meta.mdb_version != MDB_VERSION)
    {
      fclose(l_db->file);
      free(l_db);
      return MDB_DATABASE_INVALIDFILE;
    }

    /* allocates and loads the B-tree of each system table */

    /* ------- .TABLES ------- */
    l_db->tables = (mdbTable*)malloc(sizeof(mdbTable));
    l_db->tables->db = l_db;
    fread(&meta, sizeof(mdbBtreeMeta), 1, l_db->file);
    ret = mdbBtreeCreate(&T, meta.order, meta.record_size, meta.key_position);
    T->key_type = &l_db->datatypes[4];
    l_db->tables->T = T;

    /* ------ .COLUMNS ------- */
    l_db->columns = (mdbTable*)malloc(sizeof(mdbTable));
    l_db->columns->db = l_db;
    fread(&meta, sizeof(mdbBtreeMeta), 1, l_db->file);
    ret = mdbBtreeCreate(&T, meta.order, meta.record_size, meta.key_position);
    T->key_type = &l_db->datatypes[4];
    l_db->columns->T = T;

    /* ------ .INDEXES ------- */
    l_db->indexes = (mdbTable*)malloc(sizeof(mdbTable));
    l_db->indexes->db = l_db;
    fread(&meta, sizeof(mdbBtreeMeta), 1, l_db->file);
    ret = mdbBtreeCreate(&T, meta.order, meta.record_size, meta.key_position);
    T->key_type = &l_db->datatypes[4];
    l_db->indexes->T = T;

    /* ------ Root nodes of the system table B-trees ------ */
    T = l_db->tables->T;
    mdbAllocateNode(&T->root,T);
    fread(T->root->data, T->nodeSize, 1, l_db->file);

    T = l_db->columns->T;
    mdbAllocateNode(&T->root,T);
    fread(T->root->data, T->nodeSize, 1, l_db->file);

    T = l_db->indexes->T;
    mdbAllocateNode(&T->root,T);
    fread(T->root->data, T->nodeSize, 1, l_db->file);
  }
  else
  {
    free(l_db);
    return MDB_DATABASE_NOFILE;
  }

  ret = mdbLoadSystemTables(l_db);

  *db = l_db;
  return MDB_DATABASE_SUCCESS;
}

/* Loads an existing MastersDB database, including header check */
int mdbCloseDatabase(mdbDatabase *db)
{
  /* saves the system table root nodes */
  fseek(db->file, 0L, SEEK_SET);
  fwrite(&(db->meta), sizeof(mdbDatabaseMeta), 1, db->file);

  /* frees all dynamically allocated structures */
  mdbFreeTable(db->tables);
  mdbFreeTable(db->columns);
  mdbFreeTable(db->indexes);

  /* the file can now be closed */
  fclose(db->file);

  free(db);

  return MDB_DATABASE_SUCCESS;
}
