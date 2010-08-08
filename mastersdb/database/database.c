/*
 * database.c
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
 */

#include "database.h"

/* TODO: implement the special comparison functions */

int mdbCompareFloat(const void* v1, const void* v2, uint32 size)
{
  return 0;
}

void mdbAllocateTable(mdbTable **table)
{
  *table = (mdbTable*)malloc(sizeof(mdbTable));

  (*table)->name = (*table)->data;
  (*table)->num_columns = (byte*)((*table)->data + 59);
  (*table)->position = (uint32*)((*table)->data + 60);
}

int mdbFreeTable(mdbTable *t)
{
  cfree(t->columns);
  free(t->T->root->data);
  free(t->T->root);
  free(t->T);
  free(t);
  return MDB_DATABASE_SUCCESS;
}

void mdbInitializeTypes(mdbDatabase *db)
{
  /* initialize the data types array */
  db->datatypes = (mdbDatatype*)calloc(MDB_TYPE_COUNT, sizeof(mdbDatatype));

  db->datatypes[0] = (mdbDatatype){ "....INT-8",  0, sizeof(byte),   &memcmp};
  *((uint32*)db->datatypes[0].name) = strlen(db->datatypes[0].name + 4);

  db->datatypes[1] = (mdbDatatype){ "....INT-16", 0, sizeof(uint16), &memcmp};
  *((uint32*)db->datatypes[1].name) = strlen(db->datatypes[1].name + 4);

  db->datatypes[2] = (mdbDatatype){ "....INT-32", 0, sizeof(uint32), &memcmp};
  *((uint32*)db->datatypes[2].name) = strlen(db->datatypes[2].name + 4);

  db->datatypes[3] = (mdbDatatype){ "....FLOAT" , 0, sizeof(float),
    &mdbCompareFloat};
  *((uint32*)db->datatypes[3].name) = strlen(db->datatypes[3].name + 4);

  db->datatypes[4] = (mdbDatatype){ "....STRING", 4, sizeof(byte),
    (CompareKeysPtr)&strncmp};
  *((uint32*)db->datatypes[4].name) = strlen(db->datatypes[4].name + 4);
}

mdbDatatype *mdbFindType(mdbDatabase* db, const char* key)
{
  uint32 len_key = *((uint32*)key);
  uint32 len_type;
  uint8 i;

  for (i=0; i < MDB_TYPE_COUNT; i++)
  {
    len_type = *((uint32*)db->datatypes[i].name);
    if (len_key < len_type) len_type = len_key;
    if (strncmp(key + 4, db->datatypes[i].name + 4, len_type) == 0)
    {
      return &(db->datatypes[i]);
    }
  }

  /* STRING is default */
  return &(db->datatypes[4]);
}

/* creates the system tables and writes them to a file */
void mdbCreateSystemTables(mdbDatabase *db)
{
  int ret;
  mdbBtree* T;
  mdbColumn* col;

  /* _TABLES - at order 964 the node size is approximately 128 KB */
  mdbAllocateTable(&db->tables);
  ret = mdbBtreeCreateTree(&T,MDB_TABLES_ORDER,sizeof(db->tables->data),0);
  mdbBtreeAllocateNode(&(T->root), T);
  *(T->root->is_leaf) = 1L;

  T->key_type = &db->datatypes[4];
  T->ReadNode = &mdbDummyReadNode;
  T->WriteNode = &mdbDummyWriteNode;
  T->DeleteNode = &mdbDummyDeleteNode;

  db->tables->db = db;
  db->tables->T = T;

  /* _COLUMNS - at order 7944 the node size is approximately 2 MB */
  mdbAllocateTable(&db->columns);
  ret = mdbBtreeCreateTree(&T,MDB_COLUMNS_ORDER,sizeof(mdbColumn),0);
  mdbBtreeAllocateNode(&(T->root), T);
  *(T->root->is_leaf) = 1L;

  T->key_type = &db->datatypes[4];
  T->ReadNode = &mdbDummyReadNode;
  T->WriteNode = &mdbDummyWriteNode;
  T->DeleteNode = &mdbDummyDeleteNode;

  db->columns->db = db;
  db->columns->T = T;

  /* _INDEXES - at order 7282 the node size is approximately 1 MB */
  mdbAllocateTable(&db->indexes);
  ret = mdbBtreeCreateTree(&T,MDB_INDEXES_ORDER,sizeof(mdbIndex),0);
  mdbBtreeAllocateNode(&(T->root), T);
  *(T->root->is_leaf) = 1L;

  T->key_type = &db->datatypes[4];
  T->ReadNode = &mdbDummyReadNode;
  T->WriteNode = &mdbDummyWriteNode;
  T->DeleteNode = &mdbDummyDeleteNode;

  db->indexes->db = db;
  db->indexes->T = T;

  /* --------- Create the system tables meta-data ------------- */

  strcpy(db->tables->name + 4, ".TABLES");
  *((uint32*)db->tables->name) = strlen(db->tables->name + 4);
  *(db->tables->num_columns) = 3;
  *(db->tables->position) = sizeof(mdbDatabaseMeta);
  mdbBtreeInsert(db->tables->data, db->tables->T);

  strcpy(db->columns->name + 4, ".COLUMNS");
  *((uint32*)db->columns->name) = strlen(db->columns->name + 4);
  *(db->columns->num_columns) = 5;
  *(db->columns->position) = sizeof(mdbDatabaseMeta) + sizeof(mdbBtreeMeta);
  mdbBtreeInsert(db->columns->data, db->tables->T);

  strcpy(db->indexes->name + 4, ".INDEXES");
  *((uint32*)db->indexes->name) = strlen(db->indexes->name + 4);
  *(db->indexes->num_columns) = 2;
  *(db->indexes->position) = sizeof(mdbDatabaseMeta) + 2*sizeof(mdbBtreeMeta);
  mdbBtreeInsert(db->indexes->data, db->tables->T);

  /* --------- Create the system table columns meta-data ------------- */

  /* --------- .TABLES columns ---------- */
  db->tables->columns = (mdbColumn*)calloc(3, sizeof(mdbColumn));

  col = &db->tables->columns[0];
  strcpy(col->id + 4, ".TABLES000");
  strcpy(col->name + 4, "NAME");
  strcpy(col->type + 4, "STRING");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  *((uint32*)col->type) = strlen(col->type + 4);
  col->length = 55L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->tables->columns[1];
  strcpy(col->id + 4, ".TABLES001");
  strcpy(col->name + 4, "NUM_COLUMNS");
  strcpy(col->type + 4, "INT-8");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  *((uint32*)col->type) = strlen(col->type + 4);
  col->length = 0L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->tables->columns[2];
  strcpy(col->id + 4, ".TABLES002");
  strcpy(col->name + 4, "B-TREE");
  strcpy(col->type + 4, "INT-32");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  *((uint32*)col->type) = strlen(col->type + 4);
  col->length = 0L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  /* --------- .COLUMNS columns ---------- */
  db->columns->columns = (mdbColumn*)calloc(5, sizeof(mdbColumn));

  col = &db->columns->columns[0];
  strcpy(col->id + 4, ".COLUMNS000");
  strcpy(col->name + 4, "ID");
  strcpy(col->type + 4, "STRING");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  *((uint32*)col->type) = strlen(col->type + 4);
  col->length = 60L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->columns->columns[1];
  strcpy(col->id + 4, ".COLUMNS001");
  strcpy(col->name + 4, "DATATYPE");
  strcpy(col->type + 4, "STRING");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  *((uint32*)col->type) = strlen(col->type + 4);
  col->length = 8L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->columns->columns[2];
  strcpy(col->id + 4, ".COLUMNS002");
  strcpy(col->name + 4, "NAME");
  strcpy(col->type + 4, "STRING");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  *((uint32*)col->type) = strlen(col->type + 4);
  col->length = 43L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->columns->columns[3];
  strcpy(col->id + 4, ".COLUMNS003");
  strcpy(col->name + 4, "INDEXED");
  strcpy(col->type + 4, "INT-8");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  *((uint32*)col->type) = strlen(col->type + 4);
  col->length = 0L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->columns->columns[4];
  strcpy(col->id + 4, ".COLUMNS004");
  strcpy(col->name + 4, "LENGTH");
  strcpy(col->type + 4, "INT-32");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  *((uint32*)col->type) = strlen(col->type + 4);
  col->length = 0L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  /* --------- .INDEXES columns ---------- */
  db->indexes->columns = (mdbColumn*)calloc(2, sizeof(mdbColumn));

  col = &db->indexes->columns[0];
  strcpy(col->id + 4, ".INDEXES000");
  strcpy(col->name + 4, "ID");
  strcpy(col->type + 4, "STRING");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  *((uint32*)col->type) = strlen(col->type + 4);
  col->length = 58L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->indexes->columns[1];
  strcpy(col->id + 4, ".INDEXES001");
  strcpy(col->name + 4, "B-TREE");
  strcpy(col->type + 4, "INT-32");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  *((uint32*)col->type) = strlen(col->type + 4);
  col->length = 0L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

}

int mdbLoadSystemTables(mdbDatabase *db)
{
  int ret;
  uint8 t, c;
  char id[16];
  const char *tbl_ids[3] = { ".TABLES", ".COLUMNS", ".INDEXES" };
  mdbTable *tbl_ptrs[3] = { db->tables, db->columns, db->indexes };

  for (t = 0; t < 3; t++)
  {
    /* loads the table meta data */
    strcpy(id + 4, tbl_ids[t]);
    *((uint32*)id) = strlen(id + 4);
    ret = mdbBtreeSearch(id, tbl_ptrs[t]->data, db->tables->T);

    /* loads the columns meta data */
    tbl_ptrs[t]->columns =
        (mdbColumn*)calloc(*(tbl_ptrs[t]->num_columns), sizeof(mdbColumn));

    for (c=0; c < *(tbl_ptrs[t]->num_columns); c++)
    {
      sprintf(id + 4, "%s%03u%c", tbl_ids[t], c, '\0');
      *((uint32*)id) = strlen(id + 4);
      ret = mdbBtreeSearch(id,
          (char*)&(tbl_ptrs[t]->columns[c]), db->columns->T);
    }

  }

  return MDB_DATABASE_SUCCESS;
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
        *(l_db->indexes->position) + sizeof(mdbBtreeMeta);

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
    mdbAllocateTable(&l_db->tables);
    T = (mdbBtree*)malloc(sizeof(mdbBtree));
    fread(&T->meta, sizeof(mdbBtreeMeta), 1, l_db->file);
    BT_CALC_NODESIZE(T);
    T->key_type = &l_db->datatypes[4];
    T->ReadNode = &mdbDummyReadNode;
    T->WriteNode = &mdbDummyWriteNode;
    T->DeleteNode = &mdbDummyDeleteNode;
    l_db->tables->db = l_db;
    l_db->tables->T = T;

    /* ------ .COLUMNS ------- */
    mdbAllocateTable(&l_db->columns);
    T = (mdbBtree*)malloc(sizeof(mdbBtree));
    fread(&T->meta, sizeof(mdbBtreeMeta), 1, l_db->file);
    BT_CALC_NODESIZE(T);
    T->key_type = &l_db->datatypes[4];
    T->ReadNode = &mdbDummyReadNode;
    T->WriteNode = &mdbDummyWriteNode;
    T->DeleteNode = &mdbDummyDeleteNode;
    l_db->columns->db = l_db;
    l_db->columns->T = T;

    /* ------ .INDEXES ------- */
    mdbAllocateTable(&l_db->indexes);
    T = (mdbBtree*)malloc(sizeof(mdbBtree));
    fread(&T->meta, sizeof(mdbBtreeMeta), 1, l_db->file);
    BT_CALC_NODESIZE(T);
    T->key_type = &l_db->datatypes[4];
    T->ReadNode = &mdbDummyReadNode;
    T->WriteNode = &mdbDummyWriteNode;
    T->DeleteNode = &mdbDummyDeleteNode;
    l_db->indexes->db = l_db;
    l_db->indexes->T = T;

    /* ------ Root nodes of the system table B-trees ------ */
    T = l_db->tables->T;
    mdbBtreeAllocateNode(&T->root,T);
    fread(T->root->data, T->nodeSize, 1, l_db->file);

    T = l_db->columns->T;
    mdbBtreeAllocateNode(&T->root,T);
    fread(T->root->data, T->nodeSize, 1, l_db->file);

    T = l_db->indexes->T;
    mdbBtreeAllocateNode(&T->root,T);
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

  fseek(db->file, db->tables->T->meta.root_position, SEEK_SET);
  fwrite(db->tables->T->root->data, db->tables->T->nodeSize, 1, db->file);

  fseek(db->file, db->columns->T->meta.root_position, SEEK_SET);
  fwrite(db->columns->T->root->data, db->columns->T->nodeSize, 1, db->file);

  fseek(db->file, db->indexes->T->meta.root_position, SEEK_SET);
  fwrite(db->indexes->T->root->data, db->indexes->T->nodeSize, 1, db->file);

  /* frees all dynamically allocated structures */
  mdbFreeTable(db->tables);
  mdbFreeTable(db->columns);
  mdbFreeTable(db->indexes);

  /* the file can now be closed */
  fclose(db->file);

  free(db);

  return MDB_DATABASE_SUCCESS;
}

/* Creates a table and stores its B-tree and root node into the database */
int mdbCreateTable(mdbDatabase *db, mdbTable *t, mdbBtree *tree)
{
  return MDB_DATABASE_SUCCESS;
}

int mdbLoadTable(mdbDatabase *db, mdbTable **t, const char *name)
{
  char key[64];
  int ret;
  uint8 c;
  mdbTable *l_t;
  mdbBtree *T;

  mdbAllocateTable(t);
  l_t = *t;

  /* load the table meta data */
  strcpy(key + 4, name);
  *((uint32*)key) = strlen(key + 4);
  ret = mdbBtreeSearch(key, l_t->data, db->tables->T);

  if (ret == MDB_BTREE_SEARCH_FOUND)
  {
    /* load the table columns meta data */
    l_t->columns = (mdbColumn*)calloc(*(l_t->num_columns), sizeof(mdbColumn));

    for (c=0; c < *(l_t->num_columns); c++)
    {
      sprintf(key + 4, "%s%03u%c", name, c, '\0');
      *((uint32*)key) = strlen(key + 4);
      ret = mdbBtreeSearch(key, (char*)&(l_t->columns[c]), db->columns->T);
    }

    /* load the table B-tree descriptor */
    T = (mdbBtree*)malloc(sizeof(mdbBtree));
    fseek(db->file, *(l_t->position), SEEK_SET);
    fread(&T->meta, sizeof(mdbBtreeMeta), 1, db->file);
    BT_CALC_NODESIZE(T);
    T->ReadNode = &mdbDummyReadNode;
    T->WriteNode = &mdbDummyWriteNode;
    T->DeleteNode = &mdbDummyDeleteNode;
    l_t->db = db;

    /* set the appropriate key data type */
    T->key_type = mdbFindType(db, l_t->columns[0].type);

    /* load the table's B-tree root node */
    mdbBtreeAllocateNode(&T->root,T);
    fseek(db->file, T->meta.root_position, SEEK_SET);
    fread(T->root->data, T->nodeSize, 1, db->file);
    l_t->T = T;
  }
  else
  {
    free(l_t);
    return MDB_DATABASE_NO_SUCH_TABLE;
  }

  return MDB_DATABASE_SUCCESS;
}
