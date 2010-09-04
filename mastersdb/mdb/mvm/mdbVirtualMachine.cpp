/*
 * mdbVirtualMachine.cpp
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
 *  Implemented CREATE TABLE specific VM instructions: NEWTBL, NEWCOL, CRTTBL.
 *  Added the HALT instruction.
 * 13.08.2010
 *  Implemented the INSERT INTO specific VM instruction: INSVAL, INSREC.
 * 15.08.2010
 *  Implemented the DESCRIBE specific instruction DSCTBL.
 * 17.08.2010
 *  Implemented a few SELECT specific instructions:
 *  - CPYCOL : CopyColumn
 *  - NXTREC : NextRecord
 *  - CPYREC : CopyRecord
 *  - JMP    : Jump
 *  - JMPF   : JumpOnFailure.
 * 18.08.2010
 *  The mdbVirtualTable structure now contains:
 *    - a map of its column names,
 *    - a vector of the column value positions in the source record.
 *  Implemented: CPYVAL : CopyValue().
 * 19.08.2010
 *  Implemented: NEWREC : NewRecord().
 * 20.08.2010
 *  Fixed an VM initialization bug.
 * 21.08.2010
 *  The results from executions are now returned by Execute().
 */

#include "mdbVirtualMachine.h"

namespace MDB
{

mdbVirtualMachine::mdbVirtualMachine(mdbDatabase *db)
{
  uint16 i;

  ip = 0;
  cp = 0;
  sp = 0;
  tp = 0;

  setlocale(LC_CTYPE, "en_US.utf8");

  memset(memory, 0, sizeof(memory));

  for (i = 0; i < MDB_VM_TABLES_SIZE; i++)
  {
    tables[i] = new mdbVirtualTable(db);
  }

  results = new mdbQueryResults(db);

  this->db = db;
}

/*
 * Resets the virtual machine and its memory.
 */
void mdbVirtualMachine::Reset()
{
  uint16 i;

  // free memory used by the VM's memory
  for (i = 0; i < MDB_VM_MEMORY_SIZE; i++)
  {
    if (memory[i] != NULL)
    {
      free(memory[i]);
    }
  }
  memset(memory, 0, sizeof(memory));

  // free memory used by the virtual tables
  for (i = 0; i < MDB_VM_TABLES_SIZE; i++)
  {
    tables[i]->Reset();
  }

  // reset all "pointers"
  tp = 0;
  ip = 0;
  cp = 0;
  sp = 0;
}

mdbVirtualMachine::~mdbVirtualMachine()
{
  uint32 i;
  // if there was a result, delete it
  delete results;

  // free memory used by the virtual tables
  for (i = 0; i < MDB_VM_TABLES_SIZE; i++)
  {
    delete tables[i];
  }
}

void mdbVirtualMachine::Decode()
{
  opcode = MVI_OPCODE(bytecode[ip]);
  data = MVI_DATA(bytecode[ip++]);

  switch ((mdbInstruction)opcode) {
    // No operation
    case NOP:     break;
    // Stack operations
    case PUSH:    Push(); break;
    case POP:     Pop(); break;
    // Table operations
    case CRTTBL:  CreateTable(); break;
    case LDTBL:   LoadTable(); break;
    case SETTBL:  SetTable(); break;
    case DSCTBL:  DescribeTable(); break;
    // Column operations
    case NEWCOL:  NewColumn(); break;
    case CPYCOL:  CopyColumn(); break;
    // Source record operations
    case INSVAL:  InsertValue(); break;
    case INSREC:  InsertRecord(); break;
    case NXTREC:  NextRecord(); break;
    case CPYREC:  CopyRecord(); break;
    case CPYVAL:  CopyValue(); break;
    case NEWREC:  NewRecord(); break;
    // VM control operations
    case JMP:     Jump(); break;
    case JMPF:    JumpOnFailure(); break;
    case HALT:    Reset(); break;
    default:
      break;
  }
}

/*
 * Adds the column with name stored in memory[DATA] and
 * following parameters on stack:
 *   optional:  memory[_pop()] = length
 *   always:    memory[_pop()] = type_indexed (16-9:type,8-1:indexed)
 *
 * to the current virtual table.
 */
void mdbVirtualMachine::NewColumn()
{
  uint16 type_indexed, length;
  uint8 type;

  type_indexed = _pop();

  // if type uses length parameter
  type = *((uint16*)memory[type_indexed])>>8;
  if (db->datatypes[type].header > 0)
  {
    length = _pop();
    tables[tp]->addColumn(memory[data],memory[type_indexed],memory[length]);
  }
  else
  {
    tables[tp]->addColumn(memory[data], memory[type_indexed], NULL);
  }
}

/*
 * Adds the value from memory[DATA] to the current virtual table.
 */
void mdbVirtualMachine::InsertValue()
{
  tables[tp]->addValue(memory[data]);
}

/*
 * Creates the current virtual table with name stored in memory[DATA]
 */
void mdbVirtualMachine::CreateTable()
{
  tables[tp]->CreateTable(memory[data]);
}

/*
 * Loads the table with name stored in memory[DATA] and stores into
 * the current virtual table.
 */
void mdbVirtualMachine::LoadTable()
{
  tables[tp]->LoadTable(memory[data]);
}

/*
 * Inserts the record of the current virtual table.
 */
void mdbVirtualMachine::InsertRecord()
{
  tables[tp]->InsertRecord();
}

/*
 * Returns the structure of a given table as a query result->
 */
void mdbVirtualMachine::DescribeTable()
{
  static const char* names[4] =
      { "\x004\0\0\0Name", "\x004\0\0\0Type",
        "\x006\0\0\0Length", "\x007\0\0\0Indexed" };
  static const uint16 type_indexed[4] =
      { 0x0401, 0x0400, 0x0200, 0x0000};
  static const uint32 lengths[4] = { 58L, 12L, 0, 0 };

  uint8 c;
  mdbColumn *col;

  // creates the result columns
  for (c = 0; c < 4; c++)
  {
    results->addColumn(names[c],(char*)&type_indexed[c],(char*)&lengths[c]);
  }

  // adds the results
  for (c = 0; c < tables[tp]->getColumnCount(); c++)
  {
    col = tables[tp]->getColumn(c);
    results->addValue(col->name);
    results->addValue((char*)db->datatypes[col->type].name);
    results->addValue((char*)&col->length);
    results->addValue((char*)&col->indexed);
    results->NewRecord();
  }
}

/*
 * Copies the column with name memory[DATA] of the current virtual table
 * to the result column store. If the name equals the asterisk sign (*)
 * all columns of the current virtual table are copied.
 */
void mdbVirtualMachine::CopyColumn()
{
  uint8 c;

  if (memory[data][4] == '*')
  {
    // copies all column meta-data of the current virtual table
    for (c = 0; c < tables[tp]->getColumnCount(); c++)
    {
      results->addColumn(tables[tp]->getColumn(c));
    }
  }
  else
  {
    // copies the specific column meta-data to the current virtual table
    results->addColumn(tables[tp]->getColumn(memory[data]));
  }
}

/*
 * Copies the current record of the tables[DATA] virtual table
 * to the result record.
 */
void mdbVirtualMachine::CopyRecord()
{
  uint8 c;
  mdbColumn *col;

  for (c = 0; c < tables[data]->getColumnCount(); c++)
  {
    col = tables[data]->getColumn(c);
    results->addValue(col->name, tables[data]->getValue(col->name));
  }
  results->NewRecord();
}

/*
 * Copies the value of the column with name memory[DATA] of the current
 * virtual table to the result column store.
 */
void mdbVirtualMachine::CopyValue()
{
  results->addValue(memory[data], tables[tp]->getValue(memory[data]));
}

/*
 * Retrieves the next record of the tables[DATA] virtual table.
 * Either MVI_SUCCESS or MVI_FAILURE is placed on stack, depending
 * on whether a record from the table was returned.
 */
void mdbVirtualMachine::NextRecord()
{
  _push(tables[data]->NextRecord() ? MVI_SUCCESS : MVI_FAILURE);
}

/*
 * Allocates a new result record. If there was a record allocated before,
 * it is added to the result records store.
 */
void mdbVirtualMachine::NewRecord()
{
  results->NewRecord();
}

} // END of NAMESPACE