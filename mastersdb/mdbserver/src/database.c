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

  /* _FIELDS - at order 7944 the node size is approximately 2 MB */
  retValue = mdbBtreeCreateTree(
      &db->fields, MDB_FIELDS_ORDER,
      sizeof(mdbField), MDB_FIELDS_KEY_SIZE, 0);

  /* _INDEXES - at order 7490 the node size is approximately 1 MB */
  retValue = mdbBtreeCreateTree(
      &db->indexes, MDB_INDEXES_ORDER,
      sizeof(mdbIndex), MDB_INDEXES_KEY_SIZE, 0);

  /* --------- _TABLES table ---------- */

  strcpy(table.name, "_TABLES"); table.name_header = strlen(table.name);
  table.num_fields = 3;
  table.btree = sizeof(mdbDatabaseMeta);
  mdbBtreeInsert((byte*)(&table), db->tables);

  /* --------- _FIELDS table ---------- */

  strcpy(table.name, "_FIELDS"); table.name_header = strlen(table.name);
  table.num_fields = 6;
  table.btree += sizeof(mdbBtreeMeta);
  mdbBtreeInsert((byte*)(&table), db->tables);

  /* --------- _INDEXES table ---------- */

  strcpy(table.name, "_INDEXES"); table.name_header = strlen(table.name);
  table.num_fields = 2;
  table.btree += sizeof(mdbBtreeMeta);
  mdbBtreeInsert((byte*)(&table), db->tables);

  db->tables->meta.root_position = table.btree + sizeof(mdbBtreeMeta);

  db->fields->meta.root_position = db->tables->meta.root_position
      + db->tables->nodeSize;

  db->indexes->meta.root_position = db->fields->meta.root_position
      + db->fields->nodeSize;

  /* --------- _TABLES fields ---------- */

  strcpy(field.id, "_TABLES000"); field.id_header = strlen(field.id);
  strcpy(field.name, "NAME"); field.name_header = strlen(field.name);
  strcpy(field.type, "CHAR-8"); field.type_header = strlen(field.type);
  field.length = 55L; field.indexed = 0;
  mdbBtreeInsert((byte*)(&field), db->fields);

  strcpy(field.id, "_TABLES001"); field.id_header = strlen(field.id);
  strcpy(field.name, "NUM_FIELDS"); field.name_header = strlen(field.name);
  strcpy(field.type, "INT-8"); field.type_header = strlen(field.type);
  field.length = 0L; field.indexed = 0;
  mdbBtreeInsert((byte*)(&field), db->fields);

  strcpy(field.id, "_TABLES002"); field.id_header = strlen(field.id);
  strcpy(field.name, "B-TREE"); field.name_header = strlen(field.name);
  strcpy(field.type, "INT-32"); field.type_header = strlen(field.type);
  field.length = 0L; field.indexed = 0;
  mdbBtreeInsert((byte*)(&field), db->fields);

  /* --------- _FIELDS fields ---------- */

  strcpy(field.id, "_FIELDS000"); field.id_header = strlen(field.id);
  strcpy(field.name, "ID"); field.name_header = strlen(field.name);
  strcpy(field.type, "CHAR-8"); field.type_header = strlen(field.type);
  field.length = 58L; field.indexed = 0;
  mdbBtreeInsert((byte*)(&field), db->fields);

  strcpy(field.id, "_FIELDS001"); field.id_header = strlen(field.id);
  strcpy(field.name, "NAME"); field.name_header = strlen(field.name);
  strcpy(field.type, "CHAR-8"); field.type_header = strlen(field.type);
  field.length = 45L; field.indexed = 0;
  mdbBtreeInsert((byte*)(&field), db->fields);

  strcpy(field.id, "_FIELDS002"); field.id_header = strlen(field.id);
  strcpy(field.name, "DATATYPE"); field.name_header = strlen(field.name);
  strcpy(field.type, "CHAR-8"); field.type_header = strlen(field.type);
  field.length = 8L; field.indexed = 0;
  mdbBtreeInsert((byte*)(&field), db->fields);

  strcpy(field.id, "_FIELDS003"); field.id_header = strlen(field.id);
  strcpy(field.name, "LENGTH"); field.name_header = strlen(field.name);
  strcpy(field.type, "INT-32"); field.type_header = strlen(field.type);
  field.length = 0L; field.indexed = 0;
  mdbBtreeInsert((byte*)(&field), db->fields);

  strcpy(field.id, "_FIELDS004"); field.id_header = strlen(field.id);
  strcpy(field.name, "INDEXED"); field.name_header = strlen(field.name);
  strcpy(field.type, "INT-8"); field.type_header = strlen(field.type);
  field.length = 0L; field.indexed = 0;
  mdbBtreeInsert((byte*)(&field), db->fields);

  /* --------- _INDEXES fields ---------- */

  strcpy(field.id, "_INDEXES000"); field.id_header = strlen(field.id);
  strcpy(field.name, "ID"); field.name_header = strlen(field.name);
  strcpy(field.type, "CHAR-8"); field.type_header = strlen(field.type);
  field.length = 58L; field.indexed = 0;
  mdbBtreeInsert((byte*)(&field), db->fields);

  strcpy(field.id, "_INDEXES001"); field.id_header = strlen(field.id);
  strcpy(field.name, "B-TREE"); field.name_header = strlen(field.name);
  strcpy(field.type, "INT-32"); field.type_header = strlen(field.type);
  field.length = 0L; field.indexed = 0;
  mdbBtreeInsert((byte*)(&field), db->fields);

}


/* Creates an empty MastersDB database */
mdbDatabase* mdbCreateDatabase(const char* filename)
{
  char buffer[256];
  mdbDatabase *db = (mdbDatabase*)malloc(sizeof(mdbDatabase));

  /* creates the MastersDB header */
  db->meta.magic_number = MDB_MAGIC_NUMBER;
  db->meta.mdb_version = MDB_VERSION;
  memset(db->meta.sys_tables, 0, 3*sizeof(uint32));
  memset(db->meta.usr_tables, 0, 16*sizeof(uint32));
  memset(db->meta.free_space, 0, 30*sizeof(mdbFreeEntry));

  /* writes the MastersDB header to a file */
  strcpy(buffer, filename);
  strcat(buffer,".mrdb");

  if ((db->file = fopen(&buffer[0], "wb")) != NULL)
  {
    mdbCreateSystemTables(db);
    fwrite(&db->meta,sizeof(mdbDatabaseMeta),1,db->file);
    fclose(db->file);
  }
  else
  {
    free(db);
    return NULL;
  }

  return db;
}
