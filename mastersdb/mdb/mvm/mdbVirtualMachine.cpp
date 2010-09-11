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
 * 06.09.2010
 *  Implemented a few SELECT .. WHERE specific instructions:
 *  - RSTTBL : ResetTable()
 *  - CMP    : Compare().
 * 10.09.2010
 *  Implemented: BOOL   : Boolean().
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
    case RSTTBL:  ResetTable(); break;
    // Column operations
    case NEWCOL:  NewColumn(); break;
    case CPYCOL:  CopyColumn(); break;
    case CMP:     Compare(); break;
    case BOOL:    Boolean(); break;
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
 * Resets the state of the virtual table at "address" DATA, so
 * that the next NXTREC instruction will start from its beginning.
 */
void mdbVirtualMachine::ResetTable()
{
  tables[data]->ResetRecords();
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
 * Performs a comparison of type "DATA" (see mdbOperationType) for the
 * two values described in the four parameters on stack:
 *
 * Compares the two columns or column and value based on following
 * parameters on stack:
 *    _pop() = left operand meta-data
 *    _pop() = left operand memory address
 *    _pop() = right operand meta-data
 *    _pop() = right operand memory address
 */
void mdbVirtualMachine::Compare()
{
  mdbDatatype *type;
  char *left_val;
  char *right_val;
  uint32 size;
  int cmpval;
  uint16 result;

  uint32 param = *((uint32*)memory[data]);

  uint16 col_right = param & 0x03FF;
  uint16 col_left  = (param >>= 10) & 0x03FF;
  uint8 tbl_right = (param >>= 10) & 0x0F;
  uint8 tbl_left = (param >>= 4) & 0x0F;
  mdbOperationType op = (mdbOperationType)((param >>= 4) & 0x07);

  // determine the value of the left operand
  type = db->datatypes +
      (tables[tbl_left]->getColumn(memory[col_left]))->type;
  left_val = tables[tbl_left]->getValue(memory[col_left]);

  // determine the value of the right operand
  right_val = (param & 0x08) ? memory[col_right] :
      tables[tbl_right]->getValue(memory[col_right]);

  // compares the two values based on their type
  if (type->header > 0)
  {
    size = *((uint32*)left_val);
    if (size > *((uint32*)right_val))
    {
      size = *((uint32*)right_val);
    }
    left_val += type->header;
    right_val += type->header;
  }
  else
  {
    size = type->size;
  }
  cmpval = type->compare(left_val, right_val, size);

  result = MVI_FAILURE;

  // determines the comparison result
  switch (op)
  {
    case MDB_LESS:
      if (cmpval < 0) result = MVI_SUCCESS;
      break;
    case MDB_GREATER:
      if (cmpval > 0) result = MVI_SUCCESS;
      break;
    case MDB_EQUAL:
      if (cmpval == 0) result = MVI_SUCCESS;
      break;
    case MDB_GREATER_OR_EQUAL:
      if (cmpval >= 0) result = MVI_SUCCESS;
      break;
    case MDB_LESS_OR_EQUAL:
      if (cmpval <= 0) result = MVI_SUCCESS;
      break;
    case MDB_NOT_EQUAL:
      if (cmpval != 0) result = MVI_SUCCESS;
      break;
    default:
      break;
  }

  _push(result);
}

/*
 * Takes the two parameters from stack and performs a Boolean
 * logical operation (DATA) on them, pushing the result on stack
 */
void mdbVirtualMachine::Boolean()
{
  uint16 p1 = _pop();
  uint16 p2 = _pop();

  switch ((mdbOperationType)data) {
    case MDB_AND:
      _push(p1 & p2);
      break;
    case MDB_OR:
      _push(p1 | p2);
      break;
    default:
      break;
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

string mdbVirtualMachine::generateVMsnapshot()
{
  string ret;
  uint16 i;
  uint32 len;
  char buf[32];

  ret.append("--------------------------------------\n");

  ret.append("Program:\n\n");
  for (i = 0; i < cp; i++)
  {
    _decode(i, ret);
  }
  ret.append("\n");

  ret.append("Data:\n\n");
  for (i = 0; i < MDB_VM_MEMORY_SIZE; i++)
  {
    if (memory[i] != NULL)
    {
      memset(buf, 0, 32);
      len = *((uint32*)memory[i]);
      sprintf(buf, "%u\t%u ", i, len);
      ret.append(buf);
      if (len > 0 && len < 32)
      {
        memset(buf, 0, 32);
        strncpy(buf, memory[i] + 4, len);
        ret.append(buf, len);
      }
      ret.append("\n");
    }
  }

  ret.append("\n--------------------------------------\n");

  return ret;
}

void mdbVirtualMachine::_decode(uint16 instr, string &s)
{
  uint8 _opcode = MVI_OPCODE(bytecode[instr]);
  uint16 _data = MVI_DATA(bytecode[instr]);
  uint32 cmp;
  char _cdata[10];
  int c;

  c = sprintf(_cdata, "%u\t", instr);
  s.append(_cdata, c);

  switch ((mdbInstruction)_opcode) {
    // No operation
    case NOP:     s.append("NOP\t"); break;
    // Stack operations
    case PUSH:    s.append("PUSH\t"); break;
    case POP:     s.append("POP\t"); break;
    // Table operations
    case CRTTBL:  s.append("CRTTBL\t"); break;
    case LDTBL:   s.append("LDTBL\t"); break;
    case SETTBL:  s.append("SETTBL\t"); break;
    case DSCTBL:  s.append("DSCTBL\t"); break;
    case RSTTBL:  s.append("RSTTBL\t"); break;
    // Column operations
    case NEWCOL:  s.append("NEWCOL\t"); break;
    case CPYCOL:  s.append("CPYCOL\t"); break;
    case CMP:
      s.append("CMP\t");
      cmp = *((uint32*)memory[_data]);
      cmp = (cmp & 0x70000000)>>28;
      switch ((mdbOperationType)cmp)
      {
        case MDB_LESS:              s.append("' <':"); break;
        case MDB_GREATER:           s.append("' >':"); break;
        case MDB_EQUAL:             s.append("' =':"); break;
        case MDB_GREATER_OR_EQUAL:  s.append("'>=':"); break;
        case MDB_LESS_OR_EQUAL:     s.append("'<=':"); break;
        case MDB_NOT_EQUAL:         s.append("'<>':"); break;
        default:
          break;
      }
      break;
        case BOOL:
          s.append("BOOL\t");
          cmp = *((uint32*)memory[_data]);
          cmp = (cmp & 0x70000000)>>28;
          switch ((mdbOperationType)cmp)
          {
            case MDB_AND:           s.append("'AND' : "); break;
            case MDB_OR:            s.append("'OR'  : "); break;
            default:
              break;
          }
          break;
    // Source record operations
    case INSVAL:  s.append("INSVAL\t"); break;
    case INSREC:  s.append("INSREC\t"); break;
    case NXTREC:  s.append("NXTREC\t"); break;
    case CPYREC:  s.append("CPYREC\t"); break;
    case CPYVAL:  s.append("CPYVAL\t"); break;
    case NEWREC:  s.append("NEWREC\t"); break;
    // VM control operations
    case JMP:     s.append("JMP\t"); break;
    case JMPF:    s.append("JMPF\t"); break;
    case HALT:    s.append("HALT\t"); break;
    default:
      break;
  }

  c = sprintf(_cdata, "%u", _data);
  s.append(_cdata, c);
  s.append("\n");
}

} // END of NAMESPACE
