/*
 * MQLParser.cpp
 *
 * MastersDB Query Language (MQL) parser implementation
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
 * 07.08.2010
 *  Initial version of file.
 */

#include "MQLParser.h"

// needed to mix C and C++ code
extern "C" {
  #include "../database/database.h"
}

#include <cstdio>

namespace MDB
{

void MQLParser::clearMetadata()
{
  metaMap::const_iterator it;

  // delete the table entries
  if (tables.size() > 0)
  {
    for(it = tables.begin(); it != tables.end(); ++it)
    {
      delete it->first;
      delete (mdbTable*)it->second;
    }
    tables.clear();
  }

  // delete the column entries
  if (columns.size() > 0)
  {
    for(it = columns.begin(); it != columns.end(); ++it)
    {
      delete it->first;
      delete (mdbColumn*)it->second;
    }
    columns.clear();
  }

//  // delete the index entries
//  if (indexes.size() > 0)
//  {
//    for(it = indexes.begin(); it != indexes.end(); ++it)
//    {
//      delete it->first;
//      delete (mdbIndex*)it->second;
//    }
//    indexes.clear();
//  }

}

MQLParser::MQLParser(void* database)
{
  db = database;
}

void MQLParser::mapMetadata()
{
  mdbDatabase *db = (mdbDatabase*)this->db;
  mdbBtreeTraversal* trvTables = new mdbBtreeTraversal;
  mdbTable *pTable = new mdbTable;
  mdbTable *cTable;
  mdbColumn *pField = new mdbColumn;
  mdbColumn *cField;
  string *strTable;
  string *strField;

  int iField;
  char id[64];

  clearMetadata();

  // for each table
  trvTables->node = NULL;//db->tables->root;
  trvTables->position = 0;
  trvTables->parent = NULL;

  while (mdbBtreeTraverse(&trvTables, (char*)pTable) != MDB_BTREE_NOTFOUND)
  {
//    // create a copy of the metadata
//    cTable = new mdbTable;
//    *cTable = *pTable;
////    strTable = new string(pTable->name, pTable->name_header);
//    tables.insert(metaPair(strTable, cTable));
//
//    // for each column of the current table
//    iField = 0;
//
//    while (iField < pTable->num_fields)
//    {
//      // find the column meta data
//      sprintf(id, "....%s%03u", strTable->c_str(), iField);
//      *((uint32*)id) = strTable->length() + 3;
//      mdbBtreeSearch(id, (char*)pField, db->fields);
//
//      // map into the columns map
//      cField = new mdbField;
//      *cField = *pField;
//      strField = new string(*strTable);
//      strField->append(".");
////      strField->append(pField->name, pField->name_header);
//      columns.insert(metaPair(strField, cField));
//
//      iField++;
//    }

  }

//  clearMetadata();
//
//  delete pTable;
//  delete pField;
}

MQLParser::~MQLParser()
{
  clearMetadata();
}

}
