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
 */

#include "MastersDBVM.h"

extern "C" {
  #include "../mastersdb.h"
}

namespace MDB
{

MastersDBVM::MastersDBVM()
{
  ip = 0;
  cp = 0;
  sp = 0;
  tp = 0;
}

void MastersDBVM::Reset()
{
  uint16 i;

  // free memory used by the VM's memory
  for (i = 0; i < MDB_VM_MEMORY_SIZE; i++)
  {
    delete memory[i];
  }

  // free memory used by the virtual tables
  for (i = 0; i < tp; i++)
  {
    if (tables[i].table != NULL)
    {
      mdbFreeTable((mdbTable*)tables[i].table);
      tables[i].table = NULL;
    }
    if (tables[i].record != NULL)
    {
      delete tables[i].record;
      tables[i].record = NULL;
    }
    if (tables[i].traversal != NULL)
    {
      delete ((mdbBtreeTraversal*)tables[i].traversal);
      tables[i].record = NULL;
    }
  }
  tp = 0;

  // reset all other pointers
  ip = 0;
  cp = 0;
  sp = 0;
}

void MastersDBVM::ClearResult()
{
  delete (mdbColumn*)result.columns;
  delete result.records;
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
    // Record (B-tree) operations
    case NEXTRC:  NextRecord(); break;
    case NEWRC:   NewRecord(); break;
    default:
      break;
  }

  ip += 2;
}

void MastersDBVM::AddInstruction(uint8_t opcode, uint16_t data)
{
  bytecode[cp++] = MVI_ENCODE(opcode,data);
  bytecode[cp++] = (uint8_t)data;
}

inline void MastersDBVM::Store(char* data, uint16_t ptr)
{
  memory[ptr] = data;
}

inline void MastersDBVM::Push()
{
  stack[sp++] = data;
}

inline void MastersDBVM::Pop()
{
  memory[data] = (char*)(new uint16_t);
  *((uint16_t*)memory[data]) = stack[--sp];
}

void MastersDBVM::AddTable()
{

}

void MastersDBVM::AddColumn()
{

}

void MastersDBVM::CreateTable()
{

}

void MastersDBVM::LoadTable()
{

}

void MastersDBVM::NextRecord()
{

}

void MastersDBVM::NewRecord()
{

}

} // END of NAMESPACE
