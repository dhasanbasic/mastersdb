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
 *    Initial version of file.
 * 14.07.2010
 *    Implemented a utility function for the system tables creation.
 * 15.07.2010
 *    Finished implementation of system tables creation utility function.
 */

#include "database.h"
#include "btree.h"

/* creates the system tables and writes them to a file */
void mdbCreateSystemTables(mdbDatabase *db)
{
  int retValue;

  mdbTable table;
  mdbField field;

  /* _TABLES - at order 964 the node size is approximately 128 KB */
  retValue = mdbBtreeCreateTree(
      &db->tables, MDB_TABLES_ORDER,
      sizeof(mdbTable), MDB_TABLES_KEY_SIZE, 0);

  mdbBtreeAllocateNode(&(db->tables->root), db->tables);
  *(db->tables->root->is_leaf) = 1L;

  db->tables->ReadNode = &mdbDummyReadNode;
  db->tables->WriteNode = &mdbDummyWriteNode;
  db->tables->DeleteNode = &mdbDummyDeleteNode;

  /* _FIELDS - at order 7944 the node size is approximately 2 MB */
  retValue = mdbBtreeCreateTree(
      &db->fields, MDB_FIELDS_ORDER,
      sizeof(mdbField), MDB_FIELDS_KEY_SIZE, 0);

  mdbBtreeAllocateNode(&(db->fields->root), db->fields);
  *(db->fields->root->is_leaf) = 1L;

  db->fields->ReadNode = &mdbDummyReadNode;
  db->fields->WriteNode = &mdbDummyWriteNode;
  db->fields->DeleteNode = &mdbDummyDeleteNode;

  /* _INDEXES - at order 7282 the node size is approximately 1 MB */
  retValue = mdbBtreeCreateTree(
      &db->indexes, MDB_INDEXES_ORDER,
      sizeof(mdbIndex), MDB_INDEXES_KEY_SIZE, 0);

  mdbBtreeAllocateNode(&(db->indexes->root), db->indexes);
  *(db->indexes->root->is_leaf) = 1L;

  db->indexes->ReadNode = &mdbDummyReadNode;
  db->indexes->WriteNode = &mdbDummyWriteNode;
  db->indexes->DeleteNode = &mdbDummyDeleteNode;

  /* --------- _TABLES table ---------- */

  strcpy(table.name, "_TABLES"); table.name_header = strlen(table.name);
  table.num_fields = 3;
  table.btree = sizeof(mdbDatabaseMeta);
  mdbBtreeInsert((char*)(&table), db->tables);

  /* --------- _FIELDS table ---------- */

  strcpy(table.name, "_FIELDS"); table.name_header = strlen(table.name);
  table.num_fields = 6;
  table.btree += sizeof(mdbBtreeMeta);
  mdbBtreeInsert((char*)(&table), db->tables);

  /* --------- _INDEXES table ---------- */

  strcpy(table.name, "_INDEXES"); table.name_header = strlen(table.name);
  table.num_fields = 2;
  table.btree += sizeof(mdbBtreeMeta);
  mdbBtreeInsert((char*)(&table), db->tables);

  /* --------- _TABLES fields ---------- */

  strcpy(field.id, "_TABLES000"); field.id_header = strlen(field.id);
  strcpy(field.name, "NAME"); field.name_header = strlen(field.name);
  strcpy(field.type, "CHAR-8"); field.type_header = strlen(field.type);
  field.length = 55L; field.indexed = 0;
  mdbBtreeInsert((char*)(&field), db->fields);

  strcpy(field.id, "_TABLES001"); field.id_header = strlen(field.id);
  strcpy(field.name, "NUM_FIELDS"); field.name_header = strlen(field.name);
  strcpy(field.type, "INT-8"); field.type_header = strlen(field.type);
  field.length = 0L; field.indexed = 0;
  mdbBtreeInsert((char*)(&field), db->fields);

  strcpy(field.id, "_TABLES002"); field.id_header = strlen(field.id);
  strcpy(field.name, "B-TREE"); field.name_header = strlen(field.name);
  strcpy(field.type, "INT-32"); field.type_header = strlen(field.type);
  field.length = 0L; field.indexed = 0;
  mdbBtreeInsert((char*)(&field), db->fields);

  /* --------- _FIELDS fields ---------- */

  strcpy(field.id, "_FIELDS000"); field.id_header = strlen(field.id);
  strcpy(field.name, "ID"); field.name_header = strlen(field.name);
  strcpy(field.type, "CHAR-8"); field.type_header = strlen(field.type);
  field.length = 60L; field.indexed = 0;
  mdbBtreeInsert((char*)(&field), db->fields);

  strcpy(field.id, "_FIELDS001"); field.id_header = strlen(field.id);
  strcpy(field.name, "DATATYPE"); field.name_header = strlen(field.name);
  strcpy(field.type, "CHAR-8"); field.type_header = strlen(field.type);
  field.length = 8L; field.indexed = 0;
  mdbBtreeInsert((char*)(&field), db->fields);

  strcpy(field.id, "_FIELDS002"); field.id_header = strlen(field.id);
  strcpy(field.name, "NAME"); field.name_header = strlen(field.name);
  strcpy(field.type, "CHAR-8"); field.type_header = strlen(field.type);
  field.length = 43L; field.indexed = 0;
  mdbBtreeInsert((char*)(&field), db->fields);

  strcpy(field.id, "_FIELDS003"); field.id_header = strlen(field.id);
  strcpy(field.name, "INDEXED"); field.name_header = strlen(field.name);
  strcpy(field.type, "INT-8"); field.type_header = strlen(field.type);
  field.length = 0L; field.indexed = 0;
  mdbBtreeInsert((char*)(&field), db->fields);

  strcpy(field.id, "_FIELDS004"); field.id_header = strlen(field.id);
  strcpy(field.name, "LENGTH"); field.name_header = strlen(field.name);
  strcpy(field.type, "INT-32"); field.type_header = strlen(field.type);
  field.length = 0L; field.indexed = 0;
  mdbBtreeInsert((char*)(&field), db->fields);

  /* --------- _INDEXES fields ---------- */

  strcpy(field.id, "_INDEXES000"); field.id_header = strlen(field.id);
  strcpy(field.name, "ID"); field.name_header = strlen(field.name);
  strcpy(field.type, "CHAR-8"); field.type_header = strlen(field.type);
  field.length = 58L; field.indexed = 0;
  mdbBtreeInsert((char*)(&field), db->fields);

  strcpy(field.id, "_INDEXES001"); field.id_header = strlen(field.id);
  strcpy(field.name, "B-TREE"); field.name_header = strlen(field.name);
  strcpy(field.type, "INT-32"); field.type_header = strlen(field.type);
  field.length = 0L; field.indexed = 0;
  mdbBtreeInsert((char*)(&field), db->fields);

}


/* Creates an empty MastersDB database */
mdbDatabase* mdbCreateDatabase(const char* filename)
{
  char buffer[256];
  mdbDatabase *db = (mdbDatabase*)malloc(sizeof(mdbDatabase));

  /* creates the MastersDB header */
  memset(&db->meta, 0, sizeof(mdbDatabaseMeta));
  db->meta.magic_number = MDB_MAGIC_NUMBER;
  db->meta.mdb_version = MDB_VERSION;

  /* writes the MastersDB header to a file */
  strcpy(buffer, filename);
  strcat(buffer,".mrdb");

  if ((db->file = fopen(&buffer[0], "wb")) != NULL)
  {
    mdbCreateSystemTables(db);

    /* positions of the database system tables (B-tree + root node) */
    db->meta.sys_tables[0] = sizeof(mdbDatabaseMeta);
    db->meta.sys_tables[1] = db->meta.sys_tables[0] + sizeof(mdbBtreeMeta);
    db->meta.sys_tables[2] = db->meta.sys_tables[1] + sizeof(mdbBtreeMeta);

    db->tables->meta.root_position =
        db->meta.sys_tables[2] + sizeof(mdbBtreeMeta);

    db->fields->meta.root_position = db->tables->meta.root_position
        + db->tables->nodeSize;

    db->indexes->meta.root_position = db->fields->meta.root_position
        + db->fields->nodeSize;

    /* writes all meta data to the empty database */
    fwrite(&db->meta,sizeof(mdbDatabaseMeta),1,db->file);
    fwrite(&db->tables->meta,sizeof(mdbBtreeMeta),1,db->file);
    fwrite(&db->fields->meta,sizeof(mdbBtreeMeta),1,db->file);
    fwrite(&db->indexes->meta,sizeof(mdbBtreeMeta),1,db->file);
    fwrite(db->tables->root->data,db->tables->nodeSize,1,db->file);
    fwrite(db->fields->root->data,db->fields->nodeSize,1,db->file);
    fwrite(db->indexes->root->data,db->indexes->nodeSize,1,db->file);
    fclose(db->file);
  }
  else
  {
    free(db);
    return NULL;
  }

  return db;
}
