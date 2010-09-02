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
 * 13.08.2010
 *  mdbLoadTable re-factoring (they name argument is now the key).
 */

#include "mdb.h"

mdbError mdbFreeTable(mdbTable *t)
{
  if (t->T != NULL)
  {
    fseek(t->db->file, t->T->meta.root_position, SEEK_SET);
    fwrite(t->T->root->data, t->T->nodeSize, 1, t->db->file);
    free(t->T->root->data);
    free(t->T->root);
    free(t->T);
  }
  cfree(t->columns);
  free(t);
  return MDB_NO_ERROR;
}

/* creates the system tables and writes them to a file */
mdbError mdbCreateSystemTables(mdbDatabase *db)
{
  static const char *tbl_names[3] = { ".Tables", ".Columns", ".Indexes" };
  static const uint32 tbl_columns[3] = {3L, 5L, 2L};
  static const uint32 tbl_recordlens[3] = {
      sizeof(mdbTableRecord), sizeof(mdbColumnRecord), sizeof(mdbIndexRecord)
  };
  static const char *col_names[3][5] = {
      { "Name", "Columns", "B-tree", "", "" },
      { "Identifier", "Type", "Indexed", "Name", "Length", },
      { "Identifier", "B+tree", "", "", "" },
  };
  static const uint8 col_types[3][5] = {
      { 4, 2, 2, 0, 0 },
      { 4, 0, 0, 4, 2 },
      { 4, 2, 0, 0, 0 },
  };
  static const uint32 col_lengths[3][5] = {
      { 58L, 0, 0, 0, 0 },
      { 61L, 0, 0, 58L, 0 },
      { 62L, 0, 0, 0, 0 },
  };

  mdbTable **tbl[3] = { &db->tables, &db->columns, &db->indexes};
  int ret;
  mdbColumnRecord *col;
  uint32 len;
  uint8 t; uint8 c;

  /* create the system table meta-data */
  for (t = 0; t < 3; t++)
  {
    /* initialize the table structure */
    (*tbl[t]) = (mdbTable*)malloc(sizeof(mdbTable));
    memset((*tbl[t]), 0, sizeof(mdbTable));
    (*tbl[t])->db = db;
    ret = mdbBtreeCreate(&(*tbl[t])->T, 0L, tbl_recordlens[t], 0L);
    ret = mdbAllocateNode(&((*tbl[t])->T->root), (*tbl[t])->T);
    *((*tbl[t])->T->root->is_leaf) = 1L;
    (*tbl[t])->T->key_type = &db->datatypes[4];

    len = strlen(tbl_names[t]);
    strcpy((*tbl[t])->rec.name + 4, tbl_names[t]);
    *((uint32*)(*tbl[t])->rec.name) = len;
    (*tbl[t])->rec.columns = tbl_columns[t];
    (*tbl[t])->rec.btree = sizeof(mdbDatabaseMeta) + t*sizeof(mdbBtreeMeta);
    ret = mdbBtreeInsert((char*)&(*tbl[t])->rec, db->tables->T);
  }

  /* create the system table columns meta-data */
  for (t = 0; t < 3; t++)
  {
    (*tbl[t])->columns = (mdbColumnRecord*)calloc((*tbl[t])->rec.columns,
        sizeof(mdbColumnRecord));

    for (c = 0; c < (*tbl[t])->rec.columns; c++)
    {
      col = (*tbl[t])->columns + c;

      col->indexed = (c == 0) ? 1 : 0;
      col->type = col_types[t][c];
      col->length = col_lengths[t][c];

      len = strlen(col_names[t][c]);
      strcpy(col->name + 4, col_names[t][c]);
      *((uint32*)col->name) = len;

      len = strlen(tbl_names[t]);
      sprintf(col->id + 4, "%s%03u", tbl_names[t], c);
      *((uint32*)col->id) = len + 3;

      ret = mdbBtreeInsert((char*)col, db->columns->T);
    }
  }

  return MDB_NO_ERROR;
}

mdbError mdbLoadSystemTables(mdbDatabase *db)
{
  int ret;
  uint8 t, c;
  char id[16];
  const char *tbl_ids[3] = { ".Tables", ".Columns", ".Indexes" };
  mdbTable *tbl_ptrs[3] = { db->tables, db->columns, db->indexes };

  for (t = 0; t < 3; t++)
  {
    /* loads the table meta data */
    strcpy(id + 4, tbl_ids[t]);
    *((uint32*)id) = strlen(id + 4);
    ret = mdbBtreeSearch(id, (char*)&tbl_ptrs[t]->rec, db->tables->T);

    /* loads the columns meta data */
    tbl_ptrs[t]->columns = (mdbColumnRecord*)calloc(tbl_ptrs[t]->rec.columns,
        sizeof(mdbColumnRecord));

    for (c=0; c < tbl_ptrs[t]->rec.columns; c++)
    {
      sprintf(id + 4, "%s%03u%c", tbl_ids[t], c, '\0');
      *((uint32*)id) = strlen(id + 4);
      ret = mdbBtreeSearch(id,
          (char*)&(tbl_ptrs[t]->columns[c]), db->columns->T);
    }

  }

  return MDB_NO_ERROR;
}

/* Creates a table and stores its B-tree and root node into the database */
mdbError mdbCreateTable(mdbTable *t)
{
  int ret;
  uint8 c;
  uint32 len = 0;

  /* calculate the record size and optimal B-tree order for it */
  for (c = 0; c < t->rec.columns; c++)
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
  *(t->T->root->is_leaf) = 1L;

  /* saves the B-tree descriptor and root node */
  fseek(t->db->file, 0L, SEEK_END);
  t->rec.btree = ftell(t->db->file);
  t->T->meta.root_position = t->rec.btree + sizeof(mdbBtreeMeta);

  fwrite(&(t->T->meta), sizeof(mdbBtreeMeta), 1, t->db->file);
  fwrite(t->T->root->data, t->T->nodeSize, 1, t->db->file);

  /* saves the table and column meta data */
  ret = mdbBtreeInsert((char*)&t->rec, t->db->tables->T);

  len = *((uint32*)t->rec.name);

  for (c = 0; c < t->rec.columns; c++)
  {
    if (c == 0)
    {
      t->columns[c].indexed = 1;
    }
    strncpy(t->columns[c].id + 4, t->rec.name + 4, len);
    sprintf(t->columns[c].id + 4 + len, "%03u", c);
    *((uint32*)t->columns[c].id) = len + 3;
    ret = mdbBtreeInsert((char*)&(t->columns[c]), t->db->columns->T);
  }

  return MDB_NO_ERROR;
}

mdbError mdbLoadTable(mdbDatabase *db, mdbTable **t, const char *name)
{
  char key[64];
  int ret;
  uint32 len;
  uint8 c;
  mdbTable *l_t;
  mdbBtree *T;
  mdbBtreeMeta meta;

  *t = (mdbTable*)malloc(sizeof(mdbTable));
  l_t = *t;

  l_t->db = db;

  /* load the table meta data */
  ret = mdbBtreeSearch(name, (char*)&l_t->rec, db->tables->T);

  if (ret == MDB_NO_ERROR)
  {
    /* load the table columns meta data */
    l_t->columns = (mdbColumnRecord*)calloc(l_t->rec.columns,
        sizeof(mdbColumnRecord));

    len = *((uint32*)l_t->rec.name);
    strncpy(key + 4, l_t->rec.name + 4, len);
    *((uint32*)key) = len + 3;

    for (c=0; c < l_t->rec.columns; c++)
    {
      sprintf(key + 4 + len, "%03u%c", c, '\0');
      ret = mdbBtreeSearch(key, (char*)(l_t->columns + c), db->columns->T);
    }

    /* load the table B-tree descriptor */
    fseek(db->file, l_t->rec.btree, SEEK_SET);
    fread(&meta, sizeof(mdbBtreeMeta), 1, db->file);
    ret = mdbBtreeCreate(&T,meta.order,meta.record_size,meta.key_position);

    /* set the appropriate key data type */
    T->meta.root_position = meta.root_position;
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
    return MDB_TABLE_NOT_FOUND;
  }

  return MDB_NO_ERROR;
}
