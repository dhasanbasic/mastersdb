/*
 * mdbtable.c
 *
 * Implementation of Masters DB table-specific functions
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
 *  Moved table-specific code from database.c.
 */

#include "mastersdb.h"

int mdbAllocateTable(mdbTable **table, mdbDatabase *db)
{
  *table = (mdbTable*)malloc(sizeof(mdbTable));

  (*table)->name = (*table)->data;
  (*table)->num_columns = (byte*)((*table)->data + 59);
  (*table)->position = (uint32*)((*table)->data + 60);

  (*table)->db = db;

  return MDB_TABLE_SUCCESS;
}

int mdbFreeTable(mdbTable *t)
{
  fseek(t->db->file, t->T->meta.root_position, SEEK_SET);
  fwrite(t->T->root->data, t->T->nodeSize, 1, t->db->file);
  cfree(t->columns);
  free(t->T->root->data);
  free(t->T->root);
  free(t->T);
  free(t);
  return MDB_TABLE_SUCCESS;
}

/* creates the system tables and writes them to a file */
int mdbCreateSystemTables(mdbDatabase *db)
{
  int ret;
  mdbBtree* T;
  mdbColumn* col;

  /* _TABLES - at order 964 the node size is approximately 128 KB */
  mdbAllocateTable(&db->tables, db);
  ret = mdbBtreeCreate(&T,0L,sizeof(db->tables->data),0L);
  mdbAllocateNode(&(T->root), T);
  *(T->root->is_leaf) = 1L;

  T->key_type = &db->datatypes[4];
  db->tables->T = T;

  /* _COLUMNS - at order 7944 the node size is approximately 2 MB */
  mdbAllocateTable(&db->columns, db);
  ret = mdbBtreeCreate(&T,0l,sizeof(mdbColumn),0L);
  mdbAllocateNode(&(T->root), T);
  *(T->root->is_leaf) = 1L;

  T->key_type = &db->datatypes[4];
  db->columns->T = T;

  /* _INDEXES - at order 7282 the node size is approximately 1 MB */
  mdbAllocateTable(&db->indexes, db);
  ret = mdbBtreeCreate(&T,0L,sizeof(mdbIndex),0L);
  mdbAllocateNode(&(T->root), T);
  *(T->root->is_leaf) = 1L;

  T->key_type = &db->datatypes[4];
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
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  col->type = 4; col->length = 55L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->tables->columns[1];
  strcpy(col->id + 4, ".TABLES001");
  strcpy(col->name + 4, "NUM_COLUMNS");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  col->type = 0; col->length = 0L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->tables->columns[2];
  strcpy(col->id + 4, ".TABLES002");
  strcpy(col->name + 4, "B-TREE");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  col->type = 2; col->length = 0L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  /* --------- .COLUMNS columns ---------- */
  db->columns->columns = (mdbColumn*)calloc(5, sizeof(mdbColumn));

  col = &db->columns->columns[0];
  strcpy(col->id + 4, ".COLUMNS000");
  strcpy(col->name + 4, "ID");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  col->type = 4; col->length = 60L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->columns->columns[1];
  strcpy(col->id + 4, ".COLUMNS001");
  strcpy(col->name + 4, "NAME");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  col->type = 4; col->length = 54L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->columns->columns[2];
  strcpy(col->id + 4, ".COLUMNS002");
  strcpy(col->name + 4, "DATATYPE");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  col->type = 0; col->length = 0L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->columns->columns[3];
  strcpy(col->id + 4, ".COLUMNS003");
  strcpy(col->name + 4, "INDEXED");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  col->type = 0; col->length = 0L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->columns->columns[4];
  strcpy(col->id + 4, ".COLUMNS004");
  strcpy(col->name + 4, "LENGTH");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  col->type = 2; col->length = 0L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  /* --------- .INDEXES columns ---------- */
  db->indexes->columns = (mdbColumn*)calloc(2, sizeof(mdbColumn));

  col = &db->indexes->columns[0];
  strcpy(col->id + 4, ".INDEXES000");
  strcpy(col->name + 4, "ID");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  col->type = 4; col->length = 60L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  col = &db->indexes->columns[1];
  strcpy(col->id + 4, ".INDEXES001");
  strcpy(col->name + 4, "B-TREE");
  *((uint32*)col->id) = strlen(col->id + 4);
  *((uint32*)col->name) = strlen(col->name + 4);
  col->type = 2; col->length = 0L; col->indexed = 0;
  mdbBtreeInsert((char*)col, db->columns->T);

  return MDB_TABLE_SUCCESS;
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

  return MDB_TABLE_SUCCESS;
}

/* Creates a table and stores its B-tree and root node into the database */
int mdbCreateTable(mdbTable *t)
{
  int ret;
  uint8 c;
  uint32 len = 0;

  /* calculate the record size and optimal B-tree order for it */
  for (c = 0; c < *(t->num_columns); c++)
  {
    if (t->db->datatypes[t->columns[c].type].header > 0)
    {
      len += t->columns[c].length +
          t->db->datatypes[t->columns[c].type].header;
    }
    else
    {
      len += t->db->datatypes[t->columns[c].type].size;
    }
  }

  ret = mdbBtreeCreate(&(t->T), 0L, len, 0L);
  ret = mdbAllocateNode(&(t->T->root), t->T);

  /* saves the B-tree descriptor and root node */
  fseek(t->db->file, 0L, SEEK_END);
  *(t->position) = ftell(t->db->file);
  t->T->meta.root_position = *(t->position) + sizeof(mdbBtreeMeta);

  fwrite(&(t->T->meta), sizeof(mdbBtreeMeta), 1, t->db->file);
  fwrite(t->T->root->data, t->T->nodeSize, 1, t->db->file);

  /* saves the table and column meta data */
  ret = mdbBtreeInsert(t->data, t->db->tables->T);

  len = *((uint32*)t->name);

  for (c = 0; c < *(t->num_columns); c++)
  {
    strncpy(t->columns[c].id + 4, t->name + 4, len);
    sprintf(t->columns[c].id + 4 + len, "%03u", c);
    *((uint32*)t->columns[c].id) = len + 3;
    ret = mdbBtreeInsert((char*)&(t->columns[c]), t->db->columns->T);
  }

  return MDB_TABLE_SUCCESS;
}

int mdbLoadTable(mdbDatabase *db, mdbTable **t, const char *name)
{
  char key[64];
  int ret;
  uint8 c;
  mdbTable *l_t;
  mdbBtree *T;
  mdbBtreeMeta meta;

  mdbAllocateTable(t, db);
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
    fseek(db->file, *(l_t->position), SEEK_SET);
    fread(&meta, sizeof(mdbBtreeMeta), 1, db->file);
    ret = mdbBtreeCreate(&T,meta.order,meta.record_size,meta.key_position);

    /* set the appropriate key data type */
    T->key_type = &(db->datatypes[l_t->columns[0].type]);

    /* load the table's B-tree root node */
    mdbAllocateNode(&T->root,T);
    fseek(db->file, T->meta.root_position, SEEK_SET);
    fread(T->root->data, T->nodeSize, 1, db->file);
    l_t->T = T;
  }
  else
  {
    free(l_t);
    return MDB_TABLE_NOTFOUND;
  }

  return MDB_TABLE_SUCCESS;
}
