/*
 * MQLSelect.cpp
 *
 * MastersDB Query Language SELECT statement (abstract syntax tree)
 *
 * Copyright (C) 2010, Dinko Hasanbasic (dinko.hasanbasic@gmail.com)
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
 * 08.09.2010
 *  Added support for processing cross table joins.
 *  Re-factoring of bytecode generation.
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
  string cTable;
  string cColumn = *column;

  char *name;

  mdbTableInfo *l_ti;
  mdbTableMapIterator t;
  mdbColumnMapResult c;

  if (table == NULL)
  {
    if (tables.size() > 0)
    {
      cTable = tables.begin()->first;
    }
    else
    {
      cTable = MDB_DEFAULT;
    }
  }
  else
  {
    cTable = *table;
  }

  // if the table name is encountered for the first tFirstname
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
  joins.clear();

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
  uint16 pTable = mdbVirtualMachine::MDB_VM_TABLES_SIZE + 1;
  uint16 pColumn;

  if (destColumns[0].second == "*")
  {
    pTable = tables[destColumns[0].first]->tp;
    pColumn = tables[destColumns[0].first]->columns[destColumns[0].second];
    // SET TABLE
    VM->AddInstruction(mdbVirtualMachine::SETTBL, pTable);
    // COPY COLUMN (to result columns)
    VM->AddInstruction(mdbVirtualMachine::CPYCOL, pColumn);
    asterisk = true;
    return;
  }

  for (c = 0; c < destColumns.size(); c++)
  {
    // retrieve the table and column name pointers
    if (pTable != tables[destColumns[c].first]->tp)
    {
      pTable = tables[destColumns[c].first]->tp;
      // SET TABLE
      VM->AddInstruction(mdbVirtualMachine::SETTBL, pTable);
    }
    pColumn = tables[destColumns[c].first]->columns[destColumns[c].second];
    // COPY COLUMN (to result columns)
    VM->AddInstruction(mdbVirtualMachine::CPYCOL, pColumn);
  }
}

/*
 * Creates a result record
 */
void MQLSelect::GenCopyResult(bool asterisk)
{
  uint8 c;
  uint16 pTable = mdbVirtualMachine::MDB_VM_TABLES_SIZE + 1;
  uint16 pColumn;

  if (asterisk)
  {
    pTable = tables.begin()->second->tp;
    // COPY RECORD of tables[DATA] to the result store
    VM->AddInstruction(mdbVirtualMachine::CPYREC, pTable);
    return;
  }

  // NEW RESULT RECORD
  VM->AddInstruction(mdbVirtualMachine::NEWREC,
      mdbVirtualMachine::MVI_SUCCESS);

  for (c = 0; c < destColumns.size(); c++)
  {
    // retrieve the table and column name pointers
    if (pTable != tables[destColumns[c].first]->tp)
    {
      pTable = tables[destColumns[c].first]->tp;
      // SET TABLE
      VM->AddInstruction(mdbVirtualMachine::SETTBL, pTable);
    }
    pColumn = tables[destColumns[c].first]->columns[destColumns[c].second];
    // COPY VALUE of column memory[DATA] (to current result columns)
    VM->AddInstruction(mdbVirtualMachine::CPYVAL, pColumn);
  }
}

void MQLSelect::GenConditionCheck(uint16 fail_address)
{
  if (where == NULL) return;

  while(where != NULL)
  {
    GenConditionCheckRecursive(where);
    where = NULL;
  }

  VM->AddInstruction(mdbVirtualMachine::JMPF, fail_address);
}

void MQLSelect::GenConditionCheckRecursive(mdbOperation *op)
{

  if (op->left_child != NULL)
  {
    GenConditionCheckRecursive(op->left_child);
  }

  if (op->right_child != NULL)
  {
    GenConditionCheckRecursive(op->right_child);
  }

  if (op->type < MDB_AND)
  {
    VM->AddInstruction(mdbVirtualMachine::CMP, op->param_addr);
  }
  else
  {
    VM->AddInstruction(mdbVirtualMachine::BOOL, op->type);
  }

  delete op;

}

/*
 * Generates the MastersDB virtual machine byte-code equivalent for the
 * parsed SELECT MQL query
 */
void MQLSelect::GenerateBytecode()
{
  uint8 level;
  set<uint8>::iterator iter;

  bool asterisk = false;

  // Phase 1 - load the tables
  // ------------------------------------------------------------------
  GenLoadTables();
  // ------------------------------------------------------------------

  // Phase 2 - Define the result table structure
  // ------------------------------------------------------------------
  GenDefineResults(asterisk);
  // ------------------------------------------------------------------

  // Phase 3 - The record retrieval loops (: Yes! It's a loop :)
  // ------------------------------------------------------------------
  if (joins.size() > 0)
  {
    // generate the loop starts
    iter = joins.begin();
    for (level = 0; level < joins.size(); iter++, level++)
    {
      if (level > 0)
      {
        VM->AddInstruction(mdbVirtualMachine::RSTTBL, *iter);
        // NEXT RECORD
      }

      loop_start[level] = VM->getCodePointer();

      // NEXT RECORD
      VM->AddInstruction(mdbVirtualMachine::NXTREC, *iter);
      // NO OPERATION (place-holder for JUMP ON FAILURE)
      VM->AddInstruction(mdbVirtualMachine::NOP,
          mdbVirtualMachine::MVI_SUCCESS);
    }

    // check WHERE conditions
    GenConditionCheck(loop_start[level-1]);

    GenCopyResult(false);

    // generate the loop ends
    level = joins.size();
    while (level--)
    {
      // JUMP to start of loop
      VM->AddInstruction(mdbVirtualMachine::JMP, loop_start[level]);

      // rewrites the NOP operation from above to be a
      // jump to the current instruction
      VM->RewriteInstruction(loop_start[level] + 1,
          mdbVirtualMachine::JMPF, VM->getCodePointer());

      // ensures that the last result record is copied to the result table
      if (level == 0)
      {
        // NEW RESULT RECORD
        VM->AddInstruction(mdbVirtualMachine::NEWREC,
            mdbVirtualMachine::MVI_SUCCESS);
        break;
      }
    }
  }
  else
  {
    // saves the loop start
    loop_start[0] = VM->getCodePointer();
    // NEXT RECORD
    VM->AddInstruction(mdbVirtualMachine::NXTREC, tables.begin()->second->tp);
    // NO OPERATION (place-holder for JUMP ON FAILURE)
    VM->AddInstruction(mdbVirtualMachine::NOP,
        mdbVirtualMachine::MVI_SUCCESS);

    // if WHERE specified, check conditions
    GenConditionCheck(loop_start[0]);

    // copy results
    GenCopyResult(asterisk);

    // JUMP to start of loop
    VM->AddInstruction(mdbVirtualMachine::JMP, loop_start[0]);

    // rewrites the NOP operation from above to be a
    // jump to the current instruction
    VM->RewriteInstruction(loop_start[0] + 1,
        mdbVirtualMachine::JMPF, VM->getCodePointer());

    if (!asterisk)
    {
      // NEW RESULT RECORD (ensures that the last result record
      //                    is copied to the result table)
      VM->AddInstruction(mdbVirtualMachine::NEWREC,
          mdbVirtualMachine::MVI_SUCCESS);
    }
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
