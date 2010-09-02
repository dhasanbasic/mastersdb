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

#include "MastersDBVM.h"

namespace MDB
{

MastersDBVM::MastersDBVM()
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
    tables[i].cp = 0;
    tables[i].record = NULL;
    tables[i].traversal = NULL;
    tables[i].table = NULL;
  }

  result = new mdbQueryResult;
  result->cp = 0;
  result->record = NULL;
  result->record_size = 0;
  result->vals.push_back(0);

  db = NULL;
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
      free(memory[i]);
    }
  }
  memset(memory, 0, sizeof(memory));

  // free memory used by the virtual tables
  for (i = 0; i < MDB_VM_TABLES_SIZE; i++)
  {
    if (tables[i].table != NULL)
    {
      mdbFreeTable((mdbTable*)tables[i].table);
      delete[] tables[i].record;
      tables[i].record = NULL;
      tables[i].vals.clear();
      tables[i].cols.clear();
      tables[i].cp = 0;
      tables[i].record_size = 0;
      tables[i].table = NULL;
      if (tables[i].traversal != NULL)
      {
        delete tables[i].traversal;
        tables[i].traversal = NULL;
      }
    }
  }

  // allocate a new result storage, if needed
  if (result == NULL)
  {
    result = new mdbQueryResult;
    result->cp = 0;
    result->record = NULL;
    result->record_size = 0;
    result->vals.push_back(0);
  }

  // reset all "pointers"
  tp = 0;
  ip = 0;
  cp = 0;
  sp = 0;
}

MastersDBVM::~MastersDBVM()
{
  uint32 i;
  // if there was a result, delete it
  if (result != NULL)
  {
    if (result->records.size() > 0)
    {
      for (i = 0; i < result->columns.size(); i++)
      {
        delete result->columns[i];
      }
      for (i = 0; i < result->records.size(); i++)
      {
        delete[] result->records[i];
      }
      delete[] result->record;
      result->vals.push_back(0);
    }
    delete result;
  }
}

void MastersDBVM::Decode()
{
  opcode = MVI_OPCODE(bytecode[ip]);
  data = MVI_DATA(bytecode[ip++]);

  switch ((mdbInstruction)opcode) {
    // No operation
    case NOP:     break;
    // Stack operations
    case PUSHM:   PushMemory(); break;
    case POPM:    PopMemory(); break;
    // Table operations
    case NEWTBL:  NewTable(); break;
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
 * Allocates a new virtual table, with name stored in memory[DATA]
 * and number of columns on top of the stack, and stores it in the
 * current virtual table.
 */
void MastersDBVM::NewTable()
{
  uint32 size = *((uint32*)memory[data]) + 4L;
  byte num_columns = stack[--sp];
  mdbTable* t;

  tables[tp].table = t = (mdbTable*)malloc(sizeof(mdbTable));
  memset(t, 0, sizeof(mdbTable));
  memcpy(t->rec.name, memory[data], size);

  t->db = db;
  t->rec.columns = num_columns;
  t->columns = (mdbColumnRecord*)calloc(num_columns, sizeof(mdbColumnRecord));
  ;
}

/*
 * Adds the mdbColumn structure from memory[DATA] to the current
 * virtual table.
 */
void MastersDBVM::NewColumn()
{
  uint32 size;
  uint8 c;
  mdbDatatype *type;

  mdbTable* t = (mdbTable*)tables[tp].table;
  memcpy(t->columns + tables[tp].cp++, memory[data], sizeof(mdbColumnRecord));
  // if this was the last column, determine the record size and
  // allocate a new record storage
  if (tables[tp].cp == tables[tp].table->rec.columns)
  {
    size = 0;
    tables[tp].vals.push_back(0);
    for (c = 0; c < tables[tp].cp; c++)
    {
      type = db->datatypes + t->columns[c].type;
      size += (type->header > 0)
          ? (t->columns[c].length * type->size + type->header) : (type->size);
      tables[tp].vals.push_back(size);
    }
    tables[tp].record_size = size;
    tables[tp].cp = 0;
    tables[tp].record = new char[size];
  }
}

/*
 * Adds the value from memory[DATA] to the current virtual table.
 */
void MastersDBVM::InsertValue()
{
  mdbTable *tbl = (mdbTable*)tables[tp].table;
  mdbDatatype *type = db->datatypes + tbl->columns[tables[tp].cp].type;
  uint32 size = (type->header > 0)
      ? (*((uint32*)memory[data]) * type->size + type->header) : (type->size);
  char *dest = tables[tp].record + tables[tp].vals[tables[tp].cp++];

  // TODO: Raise error if value size > column maximum length
  memcpy(dest, memory[data], size);

  // if this was the last column, go back to the first
  if (tables[tp].cp == tbl->rec.columns)
  {
    tables[tp].cp = 0;
  }
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
  int ret;
  uint8 c;
  uint32 size;
  mdbDatatype *type;
  mdbColumnRecord *col;
  string name;

  ret = mdbLoadTable(db, &tables[tp].table, memory[data]);

  // allocates the record storage
  tables[tp].record_size = tables[tp].table->T->meta.record_size;
  tables[tp].record = new char[tables[tp].record_size];
  memset(tables[tp].record, 0, tables[tp].record_size);

  // maps the column names to their indexes and calculates the value positions
  size = 0;
  tables[tp].vals.push_back(0);
  for (c = 0; c < tables[tp].table->rec.columns; c++)
  {
    col = tables[tp].table->columns + c;
    type = db->datatypes + col->type;
    // map the column name to its index
    name = string(col->name + 4, *((uint32*)col->name));
    tables[tp].cols[name] = c;
    // calculate the value position of the next column
    size += (type->header > 0)
        ? (col->length * type->size + type->header) : (type->size);
    tables[tp].vals.push_back(size);
  }
}

/*
 * Inserts the record of the current virtual table.
 */
void MastersDBVM::InsertRecord()
{
  int ret;
  ret = mdbBtreeInsert(tables[tp].record, tables[tp].table->T);
}

/*
 * Returns the structure of a given table as a query result->
 */
void MastersDBVM::DescribeTable()
{
  static const char* desc_names[4] = { "Name", "Type", "Length", "Indexed" };
  static const byte desc_types[4] = { 4, 4, 2, 0 };
  static const uint32 desc_lengths[4] = { 58L, 12L, 0, 0 };

  uint8 r;
  uint8 c;
  uint32 len;
  mdbColumnRecord *col;
  mdbDatatype *type;

  // creates the result columns
  for (c = 0; c < 4; c++)
  {
    col = new mdbColumnRecord;
    len = strlen(desc_names[c]);
    strncpy(col->name + 4, desc_names[c], len);
    *((uint32*)col->name) = len;
    col->type = desc_types[c];
    col->length = desc_lengths[c];
    result->columns.push_back(col);
  }

  // determine result record size
  len = 0;
  for (c = 0; c < 4; c++)
  {
    col = result->columns[c];
    type = db->datatypes + col->type;
    len += (type->header > 0) ?
        (col->length * type->size + type->header) : (type->size);
    result->vals.push_back(len);
  }

  result->record_size = len;
  // adds the results
  for (r = 0; r < tables[tp].table->rec.columns; r++)
  {
    result->cp = 0;
    result->record = new char[result->record_size];
    memset(result->record, 0, result->record_size);
    col = tables[tp].table->columns + r;
    type = db->datatypes + col->type;

    // insert the column name
    memcpy(result->record + result->vals[result->cp++],
        col->name, *((uint32*)col->name) + 4L);

    // insert the column data type name
    len = strlen(type->name);
    *((uint32*)(result->record + result->vals[result->cp])) = len;
    strncpy(result->record + result->vals[result->cp++] + 4, type->name, len);

    // insert the column length information
    *((uint32*)(result->record + result->vals[result->cp++])) = col->length;

    // insert the column indexed information
    *((byte*)(result->record + result->vals[result->cp++])) = col->indexed;

    result->records.push_back(result->record);
    result->record = NULL;
    result->cp = 0;
  }
}

/*
 * Copies the column with name memory[DATA] of the current virtual table
 * to the result column store. If the name equals the asterisk sign (*)
 * all columns of the current virtual table are copied.
 */
void MastersDBVM::CopyColumn()
{
  uint8 i;
  uint32 size = 0;
  string name = string(memory[data] + 4, *((uint32*)memory[data]));
  mdbColumnRecord *src, *dest;
  mdbDatatype *type;

  if (name == "*")
  {
    // copies all column meta-data
    for (i = 0; i < tables[tp].table->rec.columns; i++)
    {
      src = tables[tp].table->columns + i;
      dest = new mdbColumnRecord;
      type = db->datatypes + src->type;

      memcpy(dest, src, sizeof(mdbColumnRecord));
      result->columns.push_back(dest);

      size += (type->header > 0)
          ? (src->length * type->size + type->header) : (type->size);
      result->vals.push_back(size);
    }
    // in this case the full result record size in known, so
    // a new result record store can be allocated
    result->record_size = size;
    result->record = new char[size];
  }
  else
  {
    // copies a single column meta-data
    src = tables[tp].table->columns + tables[tp].cols[name];
    dest = new mdbColumnRecord;
    type = db->datatypes + src->type;
    memcpy(dest, src, sizeof(mdbColumnRecord));
    result->columns.push_back(dest);

    size = result->vals[result->cp++] + ((type->header > 0)
        ? (src->length * type->size + type->header) : (type->size));
    result->vals.push_back(size);
    result->record_size = size;
  }
}

/*
 * Copies the current record of the tables[DATA] virtual table
 * to the result record.
 */
void MastersDBVM::CopyRecord()
{
  memcpy(result->record, tables[data].record, result->record_size);
  result->records.push_back(result->record);
  result->record = new char[result->record_size];
  result->cp = 0;
}

/*
 * Copies the value of the column with name memory[DATA] of the current
 * virtual table to the result column store.
 */
void MastersDBVM::CopyValue()
{
  uint32 size = 0;
  string name = string(memory[data] + 4, *((uint32*)memory[data]));
  uint32 c = tables[tp].cols[name];
  mdbColumnRecord *col = tables[tp].table->columns + c;
  mdbDatatype *type = db->datatypes + col->type;

  size = ((type->header > 0)
      ? (col->length * type->size + type->header) : (type->size));
  memcpy(result->record + result->vals[result->cp++],
      tables[tp].record + tables[tp].vals[c], size);
}

/*
 * Retrieves the next record of the tables[DATA] virtual table.
 * Either MVI_SUCCESS or MVI_FAILURE is placed on stack, depending
 * on whether a record from the table was returned.
 */
void MastersDBVM::NextRecord()
{
  int ret;
  mdbBtreeTraversal *tr;

  // if this is the first call...
  if (tables[tp].traversal == NULL)
  {
    tr = new mdbBtreeTraversal;
    tr->node = tables[tp].table->T->root;
    tr->parent = NULL;
    tr->position = 0;
    tables[tp].traversal = tr;
  }

  ret = mdbBtreeTraverse(&tables[tp].traversal, tables[tp].record);
  _push((ret != MDB_BTREE_KEY_NOT_FOUND) ? MVI_SUCCESS : MVI_FAILURE);
}

/*
 * Allocates a new result record. If there was a record allocated before,
 * it is added to the result records store.
 */
void MastersDBVM::NewRecord()
{
  if (result->record != NULL)
  {
    result->records.push_back(result->record);
  }
  result->record = new char[result->record_size];
  result->cp = 0;
}

} // END of NAMESPACE
