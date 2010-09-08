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
 * 04.09.2010
 *  Implemented WHERE conditions support.
 */

#include "MQLSelect.h"

namespace MDB
{

MQLSelect::MQLSelect()
{
  MDB_DEFAULT = ".Default";
  where = NULL;
}

/*
 * Maps a column identifier to a data pointer.
 */
void MQLSelect::MapColumn(
    string *column,
    string *table,
    uint16 &dp,
    mdbTableInfo* &ti,
    bool destination)
{
  string cTable = (table == NULL) ? MDB_DEFAULT : (*table);
  string cColumn = *column;

  char *name;

  mdbTableInfo *l_ti;
  mdbTableMapIterator t;
  mdbColumnMapResult c;

  // if the table name is encountered for the first time
  if ((t = tables.find(cTable)) == tables.end())
  {
    // creates a new table info
    l_ti = new mdbTableInfo;

    // stores the table name in the VM memory
    l_ti->dp = dp;
    l_ti->tp = tables.size();

    if (cTable != MDB_DEFAULT)
    {
      name = (char*)malloc(cTable.length() + 4);
      cTable.copy(name + 4, cTable.length());
      *((uint32*)name) = cTable.length();
      VM->StoreData(name, dp++);
    }

    // stores the column name in the VM memory
    l_ti->columns[cColumn] = dp;
    l_ti->cdp = dp;

    name = (char*)malloc(cColumn.length() + 4);
    cColumn.copy(name + 4, cColumn.length());
    *((uint32*)name) = cColumn.length();
    VM->StoreData(name, dp++);

    tables[cTable] = l_ti;
    ti = l_ti;
  }
  // otherwise, insert the column name in the appropriate column map
  else
  {
    c = t->second->columns.insert(mdbColumnMapPair(cColumn, dp));
    // if the insert succeeded, store the column name in the VM memory
    if (c.second)
    {
      name = (char*)malloc(cColumn.length() + 4);
      cColumn.copy(name + 4, cColumn.length());
      *((uint32*)name) = cColumn.length();
      VM->StoreData(name, dp++);
    }
    ti = t->second;
    ti->cdp = c.first->second;
  }

  if (destination)
  {
    destColumns.push_back(mdbDestinationColumn(cTable, cColumn));
  }
}

/*
 * Resolves the default table
 */
void MQLSelect::ResolveDefaultTable(string *name, uint16 &dp)
{
  mdbTableMapIterator iter;
  uint16 i;
  string cTable = *name;
  char *data;

  iter = tables.begin();
  if (iter->first == MDB_DEFAULT)
  {
    tables[cTable] = iter->second;

    iter->second->dp = dp;
    data = (char*)malloc(cTable.length() + 4);
    cTable.copy(data + 4, cTable.length());
    *((uint32*)data) = cTable.length();
    VM->StoreData(data, dp++);

    iter->second->tp++;

    while (++iter != tables.end())
    {
      iter->second->tp--;
    }
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

void MQLSelect::Reset()
{
  mdbTableMapIterator t;

  for (t = tables.begin(); t != tables.end(); t++)
  {
    delete t->second;
  }
  tables.clear();
  destColumns.clear();

  if (where != NULL) {
    delete where;
  }
}

/*
 * Generates the byte code for loading all tables
 */
void MQLSelect::GenLoadTables()
{
  mdbTableMapIterator iter;

  // Phase 1 - load the tables
  // ------------------------------------------------------------------
  for (iter = tables.begin(); iter != tables.end(); iter++)
  {
    // SET TABLE
    VM->AddInstruction(mdbVirtualMachine::SETTBL, iter->second->tp);
    // LOAD TABLE with name memory[DATA]
    VM->AddInstruction(mdbVirtualMachine::LDTBL, iter->second->dp);
  }
}

/*
 * Defines the destination columns based on the selected columns
 */
void MQLSelect::GenDefineResults(bool &asterisk)
{
  uint8 c;
  uint16 pTable, pColumn;

  for (c = 0; c < destColumns.size(); c++)
  {
    if (destColumns[c].second == "*") asterisk = true;
    // retrieve the table and column name pointers
    pTable = tables[destColumns[c].first]->tp;
    pColumn = tables[destColumns[c].first]->columns[destColumns[c].second];
    // SET TABLE
    VM->AddInstruction(mdbVirtualMachine::SETTBL, pTable);
    // COPY COLUMN (to result columns)
    VM->AddInstruction(mdbVirtualMachine::CPYCOL, pColumn);
  }
}

void MQLSelect::GenTableLoop()
{

}

/*
 * Generates the MastersDB virtual machine byte-code equivalent for the
 * parsed SELECT MQL query
 */
void MQLSelect::GenerateBytecode()
{
  mdbTableMapIterator iter = tables.begin();
  uint16 jmp, fail;
  uint8 c;
  uint16 pTable, pColumn;
  bool asterisk = false;

  // Phase 1 - load the tables
  // ------------------------------------------------------------------
  GenLoadTables();
  // ------------------------------------------------------------------

  // Phase 2 - Define the result table structure
  // ------------------------------------------------------------------
  GenDefineResults(asterisk);
  // ------------------------------------------------------------------

  // Phase 3 - The record retrieval loop (: Yes! It's a loop :)
  // ------------------------------------------------------------------
  // saves the current code pointer
  jmp = VM->getCodePointer();

  // NEXT RECORD
  VM->AddInstruction(mdbVirtualMachine::NXTREC, iter->second->tp);

  // NO OPERATION (place-holder for JUMP ON FAILURE)
  fail = VM->getCodePointer();
  VM->AddInstruction(mdbVirtualMachine::NOP, mdbVirtualMachine::MVI_NOP);

  // if '*' as column name was specified
  if (asterisk)
  {
    // COPY RECORD of tables[DATA] to the result store
    VM->AddInstruction(mdbVirtualMachine::CPYREC, iter->second->tp);
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
      pTable = tables[destColumns[c].first]->tp;
      pColumn = tables[destColumns[c].first]->columns[destColumns[c].second];
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
