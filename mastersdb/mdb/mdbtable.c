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

/* creates the system tables and writes them to a file */
mdbError mdbCreateSystemTables(mdbDatabase *db)
{
  static const char *tbl_names[3] = { ".Tables", ".Columns", ".Indexes" };
  static const uint32 tbl_columns[3] = {3L, 5L, 2L};
  static const uint32 tbl_recordlens[3] = {
      sizeof(mdbTable), sizeof(mdbColumn), sizeof(mdbIndex)
  };
  static const char *col_names[3][5] = {
      { "Name", "Columns", "B-tree", "", "" },
      { "Identifier", "Type", "Indexed", "Name", "Length", },
      { "Identifier", "B+tree", "", "", "" },
  };
  static const uint8 col_types[3][5] = {
      { 4, 0, 2, 0, 0 },
      { 4, 0, 0, 4, 2 },
      { 4, 2, 0, 0, 0 },
  };
  static const uint32 col_lengths[3][5] = {
      { 55L, 0, 0, 0, 0 },
      { 58L, 0, 0, 56L, 0 },
      { 56L, 0, 0, 0, 0 },
  };

  mdbBtree **tbl[3] = { &db->tables, &db->columns, &db->indexes};
  mdbTable tbl_rec[3];

  mdbError ret;
  mdbColumn *col;
  uint32 len;
  uint8 t; uint8 c;

  /* reserve the file */


  /* create the system table meta-data */
  for (t = 0; t < 3; t++)
  {
    /* initialize the table structure */
    ret = mdbBtreeCreate(tbl[t], 0L, tbl_recordlens[t], 0L);
    (*(tbl[t]))->file = db->file;
    ret = mdbAllocateNode(&((*tbl[t])->root), (*tbl[t]));

    *((*tbl[t])->root->is_leaf) = 1L;
    (*tbl[t])->key_type = &db->datatypes[4];

    len = strlen(tbl_names[t]);
    strcpy(tbl_rec[t].name + 4, tbl_names[t]);
    *((uint32*)tbl_rec[t].name) = len;
    tbl_rec[t].columns = tbl_columns[t];
    tbl_rec[t].btree = sizeof(mdbDatabaseMeta) + t * sizeof(mdbBtreeMeta);
    ret = mdbBtreeInsert((char*)&tbl_rec[t], db->tables);
  }

  /* create the system table columns meta-data */
  col = (mdbColumn*)malloc(sizeof(mdbColumn));
  memset(col, 0, sizeof(mdbColumn));

  for (t = 0; t < 3; t++)
  {
    for (c = 0; c < tbl_rec[t].columns; c++)
    {
      col->indexed = (c == 0) ? 1 : 0;
      col->type = col_types[t][c];
      col->length = col_lengths[t][c];

      len = strlen(col_names[t][c]);
      strcpy(col->name + 4, col_names[t][c]);
      *((uint32*)col->name) = len;

      len = strlen(tbl_names[t]);
      sprintf(col->id + 4, "%s%03u", tbl_names[t], c);
      *((uint32*)col->id) = len + 3;

      ret = mdbBtreeInsert((char*)col, db->columns);
    }
  }

  free(col);

  return MDB_NO_ERROR;
}

/* Creates a table and stores its B-tree and root node in the database */
mdbError mdbCreateTable(
    mdbDatabase *db,
    const char *name,
    uint8 num_columns,
    uint32 record_size,
    mdbBtree** btree,
    void *cls,
    mdbColumnRetrievalPtr cb)
{
  mdbError ret;
  uint8 c;
  mdbColumn *col;
  uint32 len = 0;
  mdbTable tbl;
  mdbBtree *T;

  /* calculate the record size and optimal B-tree order for it */
  ret = mdbBtreeCreate(&T, 0L, record_size, 0L);
  ret = mdbAllocateNode(&(T->root), T);
  *(T->root->is_leaf) = 1L;

  /* saves the B-tree descriptor and root node */
  fseek(db->file, 0L, SEEK_END);
  tbl.btree = ftell(db->file);
  T->meta.root_position = tbl.btree + sizeof(mdbBtreeMeta);

  fwrite(&(T->meta), sizeof(mdbBtreeMeta), 1, db->file);
  fwrite(T->root->data, T->nodeSize, 1, db->file);

  /* saves the table and column meta data */
  tbl.columns = num_columns;
  memcpy(tbl.name, name, *((uint32*)name) + 4);
  ret = mdbBtreeInsert((char*)&tbl, db->tables);

  len = *((uint32*)name);

  for (c = 0; c < num_columns; c++)
  {
    col = cb(c, cls);
    strncpy(col->id + 4, name + 4, len);
    sprintf(col->id + 4 + len, "%03u", c);
    *((uint32*)col->id) = len + 3;
    ret = mdbBtreeInsert((char*)col, db->columns);
  }

  T->file = db->file;
  *btree = T;

  return MDB_NO_ERROR;
}

mdbError mdbLoadTable(
    mdbDatabase *db,
    const char *name,
    mdbBtree **btree,
    void *cls,
    mdbColumnCallbackPtr cb)
{
  char key[64];
  mdbError ret;
  uint32 len;
  uint8 c;
  uint8 key_type;
  mdbTable tbl;
  mdbColumn col;
  mdbBtree *T;
  mdbBtreeMeta meta;

  /* load the table meta data */
  ret = mdbBtreeSearch(name, (char*)&tbl, db->tables);

  if (ret == MDB_NO_ERROR)
  {
    len = *((uint32*)tbl.name);
    strncpy(key + 4, tbl.name + 4, len);
    *((uint32*)key) = len + 3;

    for (c = 0; c < tbl.columns; c++)
    {
      sprintf(key + 4 + len, "%03u%c", c, '\0');
      ret = mdbBtreeSearch(key, (char*)&col, db->columns);

      /* column call-back */
      cb(&col, cls);

      if (c == 0)
      {
        key_type = col.type;
      }
    }

    /* load the table B-tree descriptor */
    fseek(db->file, tbl.btree, SEEK_SET);
    ret = fread(&meta, sizeof(mdbBtreeMeta), 1, db->file);
    ret = mdbBtreeCreate(&T,meta.order,meta.record_size,meta.key_position);

    /* initialize the mdbBtree structure */
    T->file = db->file;
    T->meta.root_position = meta.root_position;
    T->key_type = &(db->datatypes[key_type]);

    /* load the table's B-tree root node */
    T->root = T->ReadNode(meta.root_position, T);

    *btree = T;
  }
  else
  {
    return MDB_TABLE_NOT_FOUND;
  }

  return MDB_NO_ERROR;
}
