/*
 * mdbVirtualTable.h
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

#ifndef MDBVIRTUALTABLE_H_
#define MDBVIRTUALTABLE_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

extern "C" {
  #include "mdb.h"
}

using namespace std;

namespace MDB
{

// Column map typedefs
typedef map<string,uint8>                 mdbColumnMap;
typedef pair<string,uint8>                mdbColumnMapPair;
typedef mdbColumnMap::iterator            mdbColumnMapIterator;
typedef pair<mdbColumnMapIterator,bool>   mdbColumnMapResult;

class mdbVirtualTable
{
private:
  mdbDatabase *db;              // pointer to MastersDB database
  vector<mdbColumn*> columns;   // the table columns
  vector<uint32> cpos;          // column value positions in the record
  mdbColumnMap cmap;            // used for mapping column names to indexes
  mdbBtree *T;                  // the table B-tree
  mdbBtreeTraversal *traversal; // used for traversing the B-tree
  uint8 cp;                     // current column
protected:
  char *record;                 // used for storing the current record
  uint32 record_size;           // size of a record
public:
  mdbVirtualTable(mdbDatabase *db);

  mdbColumn* getColumn(uint8 c)
  {
    return columns[c];
  }

  mdbColumn* getColumn(char *col_name)
  {
    return columns[cmap[string(col_name + 4, *((uint32*)col_name))]];
  }

  uint32 getColumnOffset(uint8 column)
  {
    return cpos[column];
  }

  void addColumn(mdbColumn *column);
  void addColumn(const char *name, char *type_indexed, char *len);

  void addValue(char *col_name, char *value);
  char* getValue(char *col_name);
  char* getValue(uint8 column);

  uint8 getColumnCount();

  void addValue(char *value);

  void LoadTable(char *name);
  void CreateTable(char *name);

  void InsertRecord();
  bool NextRecord();

  void ResetRecords();

  void Reset();

  virtual ~mdbVirtualTable();
};

}

#endif /* MDBVIRTUALTABLE_H_ */
