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
 * 18.08.2010
 *  No more need for the allColumns flag.
 *  GenSingleTableSelect() re-factoring.
 */

#include "MQLSelect.h"

namespace MDB
{

MQLSelect::MQLSelect()
{
  MDB_DEFAULT = ".Default";
}

/*
 * Maps a column identifier to a data pointer.
 */
bool MQLSelect::MapColumn(string *column, string *table, uint16 dp)
{
  string cTable = (table == NULL) ? MDB_DEFAULT : (*table);
  string cColumn = *column;
  mdbCMap *columns;

  char *name;

  mdbTMapIter t;
  mdbCMapResult c;

  // if the table name is encountered for the first time
  if ((t = tables.find(cTable)) == tables.end())
  {
    // create a new column map
    columns = new mdbCMap();
    (*columns)[cColumn] = dp;
    // store the column name in the VM memory
    name = (char*)malloc(cColumn.length() + 4);
    cColumn.copy(name + 4, cColumn.length());
    *((uint32*)name) = cColumn.length();
    VM->StoreData(name, dp);
    // store the new column map in the table
    tables[cTable] = mdbTMapItem(0, columns);
  }
  // otherwise, insert the column name in the appropriate column map
  else
  {
    columns = t->second.second;
    c = columns->insert(mdbCMapPair(cColumn, dp));
    // if the insert succeeded, store the column name in the VM memory
    if (c.second)
    {
      name = (char*)malloc(cColumn.length() + 4);
      cColumn.copy(name + 4, cColumn.length());
      *((uint32*)name) = cColumn.length();
      VM->StoreData(name, dp);
    }
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

  if (tables.size() == 1)
  {
    iter = tables.begin();
    if (iter->first == MDB_DEFAULT)
    {
      ti = tables[MDB_DEFAULT];
      ti.first = tp;
      tables[cTable] = ti;
      tables.erase(MDB_DEFAULT);
    }
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

void MQLSelect::Reset()
{
  mdbTMapIter t;

  for (t = tables.begin(); t != tables.end(); t++)
  {
    delete t->second.second;
  }
  tables.clear();
  destColumns.clear();
}

/*
 * Generates the MastersDB virtual machine byte-code equivalent for the
 * parsed SELECT MQL query
 */
void MQLSelect::GenerateBytecode()
{
  // TODO: add check for if there was a WHERE clause.
  GenSingleTableSelect();
}

/*
 * Generates the MVI byte-code for "SELECT * FROM table;"
 */
void MQLSelect::GenSingleTableSelect()
{
  uint8 c;
  mdbTMapIter iter;
  uint32 len;
  uint16 jmp, fail;
  uint16 pTable, pColumn;
  bool asterisk = false;

  char *name;

  // Phase 1 - load the table
  // ------------------------------------------------------------------
  iter = tables.begin();
  // SET TABLE
  VM->AddInstruction(mdbVirtualMachine::SETTBL, iter->second.first);
  // store the table name
  len = iter->first.length();
  name = (char*) malloc(len + 4);
  iter->first.copy(name + 4, len);
  *((uint32*)name) = len;
  VM->StoreData(name, dptr);
  // LOAD TABLE with name memory[DATA]
  VM->AddInstruction(mdbVirtualMachine::LDTBL, dptr++);
  // ------------------------------------------------------------------

  // Phase 2 - Define the destination columns
  // based on the selected columns
  // ------------------------------------------------------------------
  for (c = 0; c < destColumns.size(); c++)
  {
    if (destColumns[c].second == "*") asterisk = true;
    // retrieve the table and column name pointers
    pTable = tables[destColumns[c].first].first;
    pColumn = tables[destColumns[c].first].second->at(destColumns[c].second);
    // SET TABLE
    VM->AddInstruction(mdbVirtualMachine::SETTBL, pTable);
    // COPY COLUMN (to result columns)
    VM->AddInstruction(mdbVirtualMachine::CPYCOL, pColumn);
  }
  // ------------------------------------------------------------------

  // Phase 3 - The record retrieval loop (: Yes! It's a loop :)
  // ------------------------------------------------------------------
  // saves the current code pointer
  jmp = VM->getCodePointer();

  // NEXT RECORD
  VM->AddInstruction(mdbVirtualMachine::NXTREC, iter->second.first);

  // NO OPERATION (place-holder for JUMP ON FAILURE)
  fail = VM->getCodePointer();
  VM->AddInstruction(mdbVirtualMachine::NOP, mdbVirtualMachine::MVI_NOP);

  // if '*' as column name was specified
  if (asterisk)
  {
    // COPY RECORD of tables[DATA] to the result store
    VM->AddInstruction(mdbVirtualMachine::CPYREC, iter->second.first);
    // JUMP to instruction
    VM->AddInstruction(mdbVirtualMachine::JMP, jmp);
    // rewrites the NOP operation from above to be a
    // jump to the current instruction
    VM->RewriteInstruction(fail, mdbVirtualMachine::JMPF, VM->getCodePointer());
  }
  // otherwise, the source data needs to by copied column by column
  else
  {
    // NEW RESULT RECORD
    VM->AddInstruction(mdbVirtualMachine::NEWREC, mdbVirtualMachine::MVI_SUCCESS);
    // copies source data of current table, column by column
    for (c = 0; c < destColumns.size(); c++)
    {
      // retrieve the table and column name pointers
      pTable = tables[destColumns[c].first].first;
      pColumn = tables[destColumns[c].first].second->at(destColumns[c].second);
      // COPY VALUE of column memory[DATA] (to current result columns)
      VM->AddInstruction(mdbVirtualMachine::CPYVAL, pColumn);
    }
    // JUMP to instruction
    VM->AddInstruction(mdbVirtualMachine::JMP, jmp);
    // rewrites the NOP operation from above to be a
    // jump to the current instruction
    VM->RewriteInstruction(fail, mdbVirtualMachine::JMPF, VM->getCodePointer());
    // NEW RESULT RECORD (this is needed to ensure that the last
    // result record is added to the result records store)
    VM->AddInstruction(mdbVirtualMachine::NEWREC, mdbVirtualMachine::MVI_SUCCESS);
  }
  // ------------------------------------------------------------------

  // the last instruction will be HALT and is added by the parser
  // VM->AddInstruction(mdbVirtualMachine::HALT, mdbVirtualMachine::MVI_SUCCESS);
}

MQLSelect::~MQLSelect()
{
  if (tables.size() > 0)
  {
    Reset();
  }
}

}
