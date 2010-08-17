/*
 * MQLSelect.cpp
 *
 * MastersDB Query Language SELECT statement (abstract syntax tree)
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
 * 16.08.2010
 *  Initial version of file.
 */

#include "MQLSelect.h"

namespace MDB
{

MQLSelect::MQLSelect()
{
  MDB_DEFAULT = ".Default";
  allColumns = false;
}

/*
 * Sets the allTables flag
 */
void MQLSelect::UseAllColumns()
{
  allColumns = true;
}

/*
 * Maps a column identifier to a data pointer.
 */
bool MQLSelect::MapColumn(string *column, string *table, uint16 dp)
{
  string cTable = (table == NULL) ? MDB_DEFAULT : (*table);
  string cColumn = *column;
  mdbCMap *columns;

  mdbTMapIter t;
  mdbCMapResult c;

  // if the table name is encountered for the first time
  if ((t = tables.find(cTable)) == tables.end())
  {
    // create a new column map
    columns = new mdbCMap();
    (*columns)[cColumn] = dp;
    tables[cTable] = mdbTMapItem(0, columns);
  }
  // otherwise, insert the column name in the appropriate column map
  else
  {
    columns = t->second.second;
    c = columns->insert(mdbCMapPair(cColumn, dp));
    destColumns.push_back(mdbDestinationColumn(cTable, cColumn));
    return (c.second);
  }

  destColumns.push_back(mdbDestinationColumn(cTable, cColumn));

  return true;
}

/*
 * Maps a table identifier to a data pointer.
 */
void MQLSelect::MapTable(string *table, uint16 tp)
{
  mdbTMapItem ti;
  mdbTMapIter iter;
  uint16 i;
  string cTable = *table;
  if (allColumns)
  {
    tables[cTable] = mdbTMapItem(tp, NULL);
    for (i = 0; i < destColumns.size(); i++)
    {
      destColumns[i].first = cTable;
    }
  }
  else
  {
    if (tables.size() == 1)
    {
      ti = tables[MDB_DEFAULT];
      ti.first = tp;
      tables[cTable] = ti;
      tables.erase(MDB_DEFAULT);
      for (i = 0; i < destColumns.size(); i++)
      {
        if (destColumns[i].first == MDB_DEFAULT)
        {
          destColumns[i].first = cTable;
        }
      }
    }
    else
    {
      iter = tables.find(cTable);
      iter->second.first = tp;
    }
  }
}

void MQLSelect::Reset()
{
  mdbTMapIter t;

  for (t = tables.begin(); t != tables.end(); t++)
  {
    delete t->second.second;
  }
  tables.clear();
  destColumns.clear();
  allColumns = false;
}

/*
 * Generates the MastersDB virtual machine byte-code equivalent for the
 * parsed SELECT MQL query
 */
void MQLSelect::GenerateBytecode()
{
  mdbTMapIter iter;
  uint32 len;
  uint16 jmp;
  char *table;

  // if '*' was specified instead of column list...
  if (allColumns)
  {
    // retrieve the one and only table and store its name in the VM memory
    mdbTMapIter iter = tables.begin();
    len = iter->first.length();
    table = new char[len + 4];
    iter->first.copy(table + 4, len);
    *((uint32*)table) = len;
    VM->Store(table, dptr);
    // USE TABLE and LOAD TABLE
    VM->AddInstruction(MastersDBVM::SETTBL, iter->second.first);
    VM->AddInstruction(MastersDBVM::LDTBL, dptr++);
    // COPY ALL COLUMNS of tables[DATA].columns
    VM->AddInstruction(MastersDBVM::LDTBL, iter->second.first);
    // NEXT RECORD (retrieves next record of tables[DATA] and pushes
    // 1 on top of stack, or 0 if there are no more records
    jmp = VM->getCodePointer();
//    VM->AddInstruction(MastersDBVM::NXTREC, iter->second.first);
//    // CONDITIONAL HALT (halt if _pop == DATA)
//    VM->AddInstruction(MastersDBVM::CHALT, 0);
//    // COPY RECORD of tables[DATA] to the result store
//    VM->AddInstruction(MastersDBVM::CPYREC, iter->second.first);
//    // ADD RECORD to results and allocate a new record store
//    VM->AddInstruction(MastersDBVM::ADDREC, iter->second.first);
//    VM->AddInstruction(MastersDBVM::JMP, jmp);
  }
}

MQLSelect::~MQLSelect()
{
  if (tables.size() > 0)
  {
    Reset();
  }
}

}
