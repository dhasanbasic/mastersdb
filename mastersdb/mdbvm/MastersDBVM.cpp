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
 *  - NEWRES : NewResult
 *  - JMP    : Jump
 *  - JMPF   : JumpOnFailure.
 * 18.08.2010
 *  The mdbVirtualTable structure now contains a map of its column names.
 */

#include "MastersDBVM.h"

namespace MDB
{

MastersDBVM::MastersDBVM(mdbDatabase *db)
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
    tables[i].cp = 0;
    tables[i].record = NULL;
    tables[i].traversal = NULL;
    tables[i].table = NULL;
  }

  result.cp = 0;
  result.record_size = 0;
  result.rp = NULL;
  result.record = NULL;
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
      free(memory[i]);
      memory[i] = NULL;
    }
  }

  // free memory used by the virtual tables
  for (i = 0; i < MDB_VM_TABLES_SIZE; i++)
  {
    if (tables[i].table != NULL)
    {
      mdbFreeTable((mdbTable*)tables[i].table);
      tables[i].table = NULL;
    }
    if (tables[i].record != NULL)
    {
      delete[] tables[i].record;
      tables[i].record = NULL;
    }
    if (tables[i].traversal != NULL)
    {
      delete tables[i].traversal;
      tables[i].traversal = NULL;
    }
    if (tables[i].cols.size() > 0)
    {
      tables[i].cols.clear();
    }
    tables[i].cp = 0;
    tables[i].rp = NULL;
  }

  // reset all pointers
  tp = 0;
  ip = 0;
  cp = 0;
  sp = 0;
}

MastersDBVM::~MastersDBVM()
{
  ClearResult();
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
    // Result operations
    case NEWRES:  NewResult(); break;
    // VM control operations
    case JMP:     Jump(); break;
    case JMPF:    JumpOnFailure(); break;
    case HALT:    Reset(); break;
    default:
      break;
  }
}

/*
 * Resets the results
 */
void MastersDBVM::ClearResult()
{
  uint32 i;
  if (result.records.size() > 0)
  {
    for (i = 0; i < result.columns.size(); i++)
    {
      delete result.columns[i];
    }
    for (i = 0; i < result.records.size(); i++)
    {
      delete[] result.records[i];
    }
    result.columns.clear();
    result.records.clear();
  }
  result.record_size = 0;
  if (result.record != NULL)
  {
    delete[] result.record;
    result.record = NULL;
  }
  result.rp = NULL;
  result.cp = 0;
}

/*
 * Allocates a new virtual table, with name stored in memory[DATA]
 * and number of columns on top of the stack, and stores it in the
 * current virtual table.
 */
void MastersDBVM::NewTable()
{
  mdbTable* t;
  uint32 size = *((uint32*)memory[data]) + 4L;
  t = (mdbTable*)malloc(sizeof(mdbTable));
  memset(t, 0, sizeof(mdbTable));
  t->db = db;
  memcpy(t->rec.name, memory[data], size);
  t->rec.columns = (byte)stack[--sp];
  t->columns =
      (mdbColumnRecord*)calloc((byte)stack[sp], sizeof(mdbColumnRecord));
  tables[tp].table = t;
}

/*
 * Adds the mdbColumn structure from memory[DATA] to the current
 * virtual table.
 */
void MastersDBVM::NewColumn()
{
  mdbTable* t = (mdbTable*)tables[tp].table;
  memcpy(&t->columns[tables[tp].cp++], memory[data], sizeof(mdbColumnRecord));
}

/*
 * Adds the value from memory[DATA] to the current virtual table.
 */
void MastersDBVM::InsertValue()
{
  mdbTable *tbl = (mdbTable*)tables[tp].table;
  mdbColumnRecord *col = tbl->columns + tables[tp].cp++;
  mdbDatatype *type = db->datatypes + col->type;
  uint32 size;

  if (type->header > 0)
  {
    size = *((uint32*)memory[data]) * type->size + type->header;
    // TODO: Raise error if size > length
    memcpy(tables[tp].rp, memory[data], size);
    tables[tp].rp += col->length + type->header;
  }
  else
  {
    memcpy(tables[tp].rp, memory[data], type->size);
    tables[tp].rp += type->size;
  }

  // if this was the last column, go back to the first
  if (tables[tp].cp == tbl->rec.columns)
  {
    tables[tp].rp = tables[tp].record;
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
  uint16 c;
  uint32 len;
  mdbColumnRecord *col;
  string name;
  ret = mdbLoadTable(db, &tables[tp].table, memory[data]);
  // allocates the record storage
  tables[tp].record = new char[tables[tp].table->T->meta.record_size];
  memset(tables[tp].record, 0, tables[tp].table->T->meta.record_size);
  tables[tp].rp = tables[tp].record;
  // maps the column names to their indexes
  for (c = 0; c < tables[tp].table->rec.columns; c++)
  {
    col = tables[tp].table->columns + c;
    len = *((uint32*)col->name);
    name = string(col->name + 4, len);
    tables[tp].cols[name] = c;
  }
}

/*
 * Inserts the record of the current virtual table.
 */
void MastersDBVM::InsertRecord()
{
  int ret;
  ret = mdbBtreeInsert(tables[tp].record, tables[tp].table->T);
  void *c = NULL;
}

/*
 * Returns the structure of a given table as a query result.
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

  // creates the result columns
  for (c = 0; c < 4; c++)
  {
    col = new mdbColumnRecord;
    len = strlen(desc_names[c]);
    strncpy(col->name + 4, desc_names[c], len);
    *((uint32*)col->name) = len;
    col->type = desc_types[c];
    col->length = desc_lengths[c];
    result.columns.push_back(col);
  }

  // determine result record size
  len = 0;
  for (c = 0; c < 4; c++)
  {
    if (db->datatypes[result.columns[c]->type].header > 0)
    {
      len += result.columns[c]->length +
          db->datatypes[result.columns[c]->type].header;
    }
    else
    {
      len += db->datatypes[result.columns[c]->type].size;
    }
  }

  result.record_size = len;

  // adds the results
  for (r = 0; r < tables[tp].table->rec.columns; r++)
  {
    result.record = new char[result.record_size];
    result.rp = result.record;
    col = tables[tp].table->columns + r;

    // insert the column name
    len = *((uint32*)col->name);
    memcpy(result.rp, col->name, len + 4);
    result.rp += result.columns[0]->length + 4;

    // insert the column data type name
    len = strlen(db->datatypes[col->type].name);
    *((uint32*)result.rp) = len;
    strncpy(result.rp + 4, db->datatypes[col->type].name, len);
    result.rp += result.columns[1]->length + 4;

    // insert the column length information
    *((uint32*)result.rp) = col->length;
    result.rp += db->datatypes[result.columns[2]->type].size;

    // insert the column indexed information
    *((byte*)result.rp) = col->indexed;

    result.records.push_back(result.record);
    result.record = NULL;
  }
}

/*
 * Copies the column[DATA] of the current virtual table
 * to the result column store. If DATA is MVI_ALL then
 * all columns of the current virtual table are copied.
 */
void MastersDBVM::CopyColumn()
{
  uint8 i;
  mdbColumnRecord *src, *dest;

  if (data < MVI_ALL)
  {
    // copies a single column meta-data
    src = tables[tp].table->columns + data;
    dest = new mdbColumnRecord;
    memcpy(dest->name, src->name, sizeof(src->name));
    dest->type = src->type;
    dest->length = src->length;
    result.columns.push_back(dest);
    // updates the result record size
    if (db->datatypes[src->type].header > 0)
    {
      result.record_size += src->length + db->datatypes[src->type].header;
    }
    else
    {
      result.record_size += db->datatypes[src->type].size;
    }
  }
  else
  {
    // copies all column meta-data
    for (i = 0; i < tables[tp].table->rec.columns; i++)
    {
      src = tables[tp].table->columns + i;
      dest = new mdbColumnRecord;
      memcpy(dest->name, src->name, sizeof(src->name));
      dest->type = src->type;
      dest->length = src->length;
      result.columns.push_back(dest);
      // updates the result record size
      if (db->datatypes[src->type].header > 0)
      {
        result.record_size += src->length + db->datatypes[src->type].header;
      }
      else
      {
        result.record_size += db->datatypes[src->type].size;
      }
    }
    // in this case the full result record size in known, so
    // a new result record store can be allocated
    result.record = new char[result.record_size];
    result.rp = result.record;
  }
}

/*
 * Adds the current result record to the results store
 * and allocates a new result record.
 */
void MastersDBVM::NewResult()
{
  if (result.record != NULL)
  {
    result.records.push_back(result.record);
  }
  result.record = new char[result.record_size];
  result.rp = result.record;
  result.cp = 0;
}

/*
 * Copies the current record of the tables[DATA] virtual table
 * to the result record.
 */
void MastersDBVM::CopyRecord()
{
  memcpy(result.record, tables[data].record, result.record_size);
  result.records.push_back(result.record);
  result.record = new char[result.record_size];
  result.rp = result.record;
  result.cp = 0;
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
  _push((ret != MDB_BTREE_NOTFOUND) ? MVI_SUCCESS : MVI_FAILURE);
}

} // END of NAMESPACE
