/*
 * MastersDBVM.cpp
 *
 * MastersDB query language interpreter virtual machine (implementation)
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
 * 10.08.2010
 *  Initial version of file.
 * 12.08.2010
 *  Implemented CREATE TABLE specific VM instructions: ADDTBL, ADDCOL, CRTTBL.
 */

#include "MastersDBVM.h"

extern "C" {
  #include "../mastersdb.h"
}

namespace MDB
{

MastersDBVM::MastersDBVM(void *db)
{
  uint16 i;

  ip = 0;
  cp = 0;
  sp = 0;
  tp = 0;

  for (i = 0; i < MDB_VM_MEMORY_SIZE; i++)
  {
    memory[i] = NULL;
  }

  for (i = 0; i < MDB_VM_TABLES_SIZE; i++)
  {
    memset(&tables[i], 0, sizeof(mdbVirtualTable));
  }

  this->db = db;
}

/*
 * Resets the virtual machine and its memory.
 */
void MastersDBVM::Reset()
{
  uint16 i;

  // free memory used by the VM's memory
  for (i = 0; i < MDB_VM_MEMORY_SIZE; i++)
  {
    if (memory[i] != NULL)
    {
      delete memory[i];
      memory[i] = NULL;
    }
  }

  // free memory used by the virtual tables
  for (i = 0; i < MDB_VM_TABLES_SIZE; i++)
  {
    if (tables[i].table != NULL)
    {
      mdbFreeTable((mdbTable*)tables[i].table);
    }
    if (tables[i].record != NULL)
    {
      delete tables[i].record;
    }
    if (tables[i].traversal != NULL)
    {
      delete ((mdbBtreeTraversal*)tables[i].traversal);
    }
    memset(&tables[i], 0, sizeof(mdbVirtualTable));
  }

  // reset all pointers
  tp = 0;
  ip = 0;
  cp = 0;
  sp = 0;
}

MastersDBVM::~MastersDBVM()
{
  Reset();
}

void MastersDBVM::Decode()
{
  opcode = MVI_OPCODE(bytecode[ip]);
  data = MVI_DATA(bytecode[ip], bytecode[ip + 1]);

  switch ((mdbInstruction)opcode) {
    // Stack operations
    case PUSH:    Push(); break;
    case POP:     Pop(); break;
    // Table operations
    case ADDTBL:  AddTable(); break;
    case ADDCOL:  AddColumn(); break;
    case CRTBL:   CreateTable(); break;
    case LDTBL:   LoadTable(); break;
    case USETBL:  UseTable(); break;
    // Record (B-tree) operations
    case NEXTRC:  NextRecord(); break;
    case NEWRC:   NewRecord(); break;
    default:
      break;
  }

  ip += 2;
}

/*
 * Resets the results
 */
void MastersDBVM::ClearResult()
{
  delete (mdbColumn*)result.columns;
  delete result.records;
  memset(&result, 0L, sizeof(mdbQueryResult));
};


/*
 * Allocates a new virtual table, with name stored in memory[DATA]
 * and number of columns on top of the stack, and stores it in the
 * current virtual table.
 */
void MastersDBVM::AddTable()
{
  int ret;
  mdbTable* t;
  uint32 size = *((uint32*)memory[data]) + 4L;
  ret = mdbAllocateTable(&t, (mdbDatabase*)db);
  memcpy(t->name, memory[data], size);
  *(t->num_columns) = (byte)stack[--sp];
  t->columns = (mdbColumn*)calloc((byte)stack[sp], sizeof(mdbColumn));
  tables[tp].table = t;
}

/*
 * Adds the mdbColumn structure from memory[DATA] to the current
 * virtual table.
 */
void MastersDBVM::AddColumn()
{
  mdbTable* t = (mdbTable*)tables[tp].table;
  memcpy(&t->columns[tables[tp].cp++], memory[data], sizeof(mdbColumn));
}

/*
 * Creates the current virtual table.
 */
void MastersDBVM::CreateTable()
{
  int ret;
  ret = mdbCreateTable((mdbTable*)tables[tp].table);
}

/*
 * Loads the table with name stored in memory[DATA] and stores into
 * the current virtual table.
 */
void MastersDBVM::LoadTable()
{

}

/*
 * Loads the next record of the current virtual table.
 */
void MastersDBVM::NextRecord()
{

}

/*
 * Allocates a new result record.
 */
void MastersDBVM::NewRecord()
{

}

} // END of NAMESPACE
