/*
 * mdbVirtualTable.cpp
 *
 * MastersDB virtual table (contains table meta-data)
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
 * 02.09.2010
 *  Initial version of file.
 * 06.09.2010
 *  Added the ResetRecords method.
 */

#include "mdbVirtualTable.h"

namespace MDB
{

void ColumnCallback(mdbColumn *col, void* cls)
{
  mdbVirtualTable *tbl = (mdbVirtualTable*)cls;
  tbl->addColumn(col);
}

mdbColumn* ColumnRetrieval(uint8 c, void* cls)
{
  mdbVirtualTable *tbl = (mdbVirtualTable*)cls;
  return tbl->getColumn(c);
}

mdbVirtualTable::mdbVirtualTable(mdbDatabase *db)
{
  this->db = db;
  T = NULL;
  traversal = NULL;
  record = NULL;
  cp = 0;
  record_size = 0;
}

mdbVirtualTable::~mdbVirtualTable()
{
  Reset();
}

void mdbVirtualTable::addColumn(mdbColumn *column)
{
  mdbColumn *col = new mdbColumn;
  memcpy(col, column, sizeof(mdbColumn));

  columns.push_back(col);
  cmap[string(col->name + 4, *((uint32*)col->name))] = columns.size() - 1;
  cpos.push_back(record_size);

  if (db->datatypes[column->type].header > 0)
  {
    record_size += db->datatypes[col->type].header +
        col->length * db->datatypes[col->type].size;
  }
  else
  {
    record_size += db->datatypes[col->type].size;
  }
}

void mdbVirtualTable::addColumn(
    const char *name,
    char *type_indexed,
    char *len)
{
  mdbColumn *col = new mdbColumn;

  memcpy(col->name, name, *((uint32*)name) + 4);
  col->type = *((uint16*)type_indexed)>>8;
  col->indexed = *((uint8*)type_indexed);

  if (len != NULL)
  {
    memcpy(&col->length, len, sizeof(uint32));
  }
  else
  {
    col->length = 0;
  }

  addColumn(col);

  delete col;
}

void mdbVirtualTable::addValue(char *col_name, char *value)
{
  uint8 c = cmap[string(col_name + 4, *((uint32*)col_name))];

  if (record == NULL) record = new char[record_size];

  if (db->datatypes[columns[c]->type].header > 0)
  {
    memcpy(record + cpos[c],
        value,
        columns[c]->length + db->datatypes[columns[c]->type].header);
  }
  else
  {
    memcpy(record + cpos[c], value, db->datatypes[columns[c]->type].size);
  }
}

char* mdbVirtualTable::getValue(char *col_name)
{
  uint8 c = cmap[string(col_name + 4, *((uint32*)col_name))];
  return (record + cpos[c]);
}

char* mdbVirtualTable::getValue(uint8 column)
{
  return (record + cpos[column]);
}

uint8 mdbVirtualTable::getColumnCount()
{
  return columns.size();
}

void mdbVirtualTable::addValue(char *value)
{
  if (record == NULL) record = new char[record_size];

  // TODO: Raise error if value size > column maximum length
  if (db->datatypes[columns[cp]->type].header > 0)
  {
    memcpy(record + cpos[cp], value, *((uint32*)value) + 4);
  }
  else
  {
    memcpy(record + cpos[cp], value, db->datatypes[columns[cp]->type].size);
  }

  cp = (cp < (columns.size()-1)) ? (cp + 1) : 0;
}

void mdbVirtualTable::LoadTable(char *name)
{
  mdbError ret;
  ret = mdbLoadTable(db, name, &T, (void*)this, &ColumnCallback);
  record = new char[record_size];
}

void mdbVirtualTable::CreateTable(char *name)
{
  mdbError ret;
  ret = mdbCreateTable(db, name, columns.size(), record_size, &T,
      (void*)this, &ColumnRetrieval);
  record = new char[record_size];
}

void mdbVirtualTable::InsertRecord()
{
  mdbError ret;
  ret = mdbBtreeInsert(record, T);
}

bool mdbVirtualTable::NextRecord()
{
  mdbError ret;

  // if this is the first call...
  if (traversal == NULL)
  {
    traversal = new mdbBtreeTraversal;
    traversal->node = T->root;
    traversal->parent = NULL;
    traversal->position = 0;
  }

  ret = mdbBtreeTraverse(&traversal, record);
  return (ret != MDB_BTREE_NO_MORE_RECORDS);
}

void mdbVirtualTable::ResetRecords()
{
  if (traversal != NULL)
  {
    while (traversal->parent != NULL)
    {
      mdbFreeNode(traversal->node, 0);
      traversal = traversal->parent;
    }
    traversal->position = 0;
  }
}

void mdbVirtualTable::Reset()
{
  uint32 c;

  if (T != NULL)
  {
    mdbFreeNode(T->root, 1);
    free(T);
  }

  if (columns.size() > 0)
  {
    for (c = 0; c < columns.size(); c++)
    {
      delete columns[c];
    }
  }

  ResetRecords();

  delete traversal;
  delete[] record;

  T = NULL;
  traversal = NULL;
  record = NULL;
  cp = 0;
  record_size = 0;
  columns.clear();
  cmap.clear();
  cpos.clear();
}

}
