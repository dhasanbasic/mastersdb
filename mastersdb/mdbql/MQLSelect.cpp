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
  // if '*' was specified instead of column list...
  if (allColumns)
  {
    GenerateSelectBytecode();
  }
}

/*
 * Generates the MVI byte-code for "SELECT * FROM table;"
 */
void MQLSelect::GenerateSelectBytecode()
{
  mdbTMapIter iter;
  uint32 len;
  uint16 jmp, fail;
  char *table;

  // retrieve the one and only table and store its name in the VM memory
  iter = tables.begin();
  len = iter->first.length();
  table = (char*)malloc(len + 4);
  iter->first.copy(table + 4, len);
  *((uint32*)table) = len;
  VM->StoreData(table, dptr);
  // USE TABLE and LOAD TABLE
  VM->AddInstruction(MastersDBVM::SETTBL, iter->second.first);
  VM->AddInstruction(MastersDBVM::LDTBL, dptr++);
  // COPY COLUMN (all columns of tables[DATA].columns to result columns)
    VM->AddInstruction(MastersDBVM::CPYCOL, MastersDBVM::MVI_ALL);
  // NEXT RECORD
  jmp = VM->getCodePointer();
  VM->AddInstruction(MastersDBVM::NXTREC, iter->second.first);
  // place-holder instruction, will be rewritten: JUMP IF FAILURE
  fail = VM->getCodePointer();
  VM->AddInstruction(MastersDBVM::NOP, MastersDBVM::MVI_NOP);
  // COPY RECORD of tables[DATA] to the result store
  VM->AddInstruction(MastersDBVM::CPYREC, iter->second.first);
  VM->AddInstruction(MastersDBVM::JMP, jmp);
  // rewrites the NOP operation from above to be a
  // jump to the current instruction
  VM->RewriteInstruction(fail, MastersDBVM::JMPF, VM->getCodePointer());
  // the last instruction will be HALT and is added
  // automatically by the parser
  // VM->AddInstruction(MastersDBVM::HALT, MastersDBVM::MVI_SUCCESS);
}

MQLSelect::~MQLSelect()
{
  if (tables.size() > 0)
  {
    Reset();
  }
}

}
