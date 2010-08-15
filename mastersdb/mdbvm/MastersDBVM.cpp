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
 *  Added the HALT instruction.
 * 13.08.2010
 *  Implemented the INSERT INTO specific VM instruction: ADDVAL, INSTBL.
 * 15.08.2010
 *  Implemented the DESCRIBE specific instruction DSCTBL.
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
    memset(&tables[i], 0, sizeof(mdbVirtualTable));
  }
  result.columns = new vector<mdbColumnRecord*>;
  result.records = new vector<char*>;
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
  delete result.columns;
  delete result.records;
}

void MastersDBVM::Decode()
{
  opcode = MVI_OPCODE(bytecode[ip]);
  data = MVI_DATA(bytecode[ip], bytecode[ip + 1]);

  switch ((mdbInstruction)opcode) {
    // Stack operations
    case PUSH:    Push(); break;
    case POP:     Pop(); break;
    case PUSHOB:  PushOffsetByte(); break;
    // Table operations
    case ADDTBL:  AddTable(); break;
    case ADDCOL:  AddColumn(); break;
    case ADDVAL:  AddValue(); break;
    case CRTBL:   CreateTable(); break;
    case LDTBL:   LoadTable(); break;
    case USETBL:  UseTable(); break;
    case INSTBL:  InsertIntoTable(); break;
    case DSCTBL:  DescribeTable(); break;
    // Result operation
    case NEWCOL:  NewResultColumn(); break;
    case NEWREC:  NewResultRecord(); break;
    // Record (B-tree) operations
    case NEXTRC:  NextRecord(); break;
    case NEWRC:   NewRecord(); break;
    // VM control operations
    case HALT:    Reset(); break;
    default:
      break;
  }

  if (opcode != HALT)
  {
    ip += 2;
  }
}

/*
 * Resets the results
 */
void MastersDBVM::ClearResult()
{
  uint32 i;
  for (i = 0; i < result.columns->size(); i++)
  {
    delete result.columns->at(i);
  }
  for (i = 0; i < result.records->size(); i++)
  {
    delete result.records->at(i);
  }
  result.columns->clear();
  result.records->clear();
  result.record_size = 0;
  result.rp = NULL;
  result.cp = 0;
};


/*
 * Allocates a new virtual table, with name stored in memory[DATA]
 * and number of columns on top of the stack, and stores it in the
 * current virtual table.
 */
void MastersDBVM::AddTable()
{
  mdbTable* t;
  uint32 size = *((uint32*)memory[data]) + 4L;
  t = (mdbTable*)malloc(sizeof(mdbTable));
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
void MastersDBVM::AddColumn()
{
  mdbTable* t = (mdbTable*)tables[tp].table;
  memcpy(&t->columns[tables[tp].cp++], memory[data], sizeof(mdbColumnRecord));
}

/*
 * Adds the value from memory[DATA] to the current virtual table.
 */
void MastersDBVM::AddValue()
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
  ret = mdbLoadTable(db, &tables[tp].table, memory[data]);
  // allocates the record storage
  tables[tp].record = new char[tables[tp].table->T->meta.record_size];
  tables[tp].rp = tables[tp].record;
}

/*
 * Inserts the record of the current virtual table.
 */
void MastersDBVM::InsertIntoTable()
{
  int ret;
  ret = mdbBtreeInsert(tables[tp].record, tables[tp].table->T);
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
  char *record;
  char *rp;
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
    result.columns->push_back(col);
  }

  // determine result record size
  len = 0;
  for (c = 0; c < 4; c++)
  {
    if (db->datatypes[result.columns->at(c)->type].header > 0)
    {
      len += result.columns->at(c)->length +
          db->datatypes[result.columns->at(c)->type].header;
    }
    else
    {
      len += db->datatypes[result.columns->at(c)->type].size;
    }
  }

  result.record_size = len;

  // add the results
  for (r = 0; r < tables[tp].table->rec.columns; r++)
  {
    record = new char[result.record_size];
    rp = record;
    col = tables[tp].table->columns + r;

    // insert the column name
    len = *((uint32*)col->name);
    memcpy(rp, col->name, len + 4);
    rp += result.columns->at(0)->length + 4;

    // insert the column data type name
    len = strlen(db->datatypes[col->type].name);
    *((uint32*)rp) = len;
    strncpy(rp + 4, db->datatypes[col->type].name, len);
    rp += result.columns->at(1)->length + 4;

    // insert the column length information
    *((uint32*)rp) = col->length;
    rp += db->datatypes[result.columns->at(2)->type].size;

    // insert the column indexed information
    *((byte*)rp) = col->indexed;

    result.records->push_back(record);
  }

}

/*
 * Creates a new virtual column for the query results, based on:
 *   column.name = memory[DATA]
 *   column.type = stack[sp-2];
 *   column.length = memory[stack[sp-1];
 */
void MastersDBVM::NewResultColumn()
{
  mdbColumnRecord *c = new mdbColumnRecord;
  // retrieves the column name
  uint32 len = *((uint32*)memory[data]);
  strncpy(c->name + 4, memory[data] + 4, len);
  *((uint32*)c->name) = len;
  // retrieves the column length
  c->length = *((uint32*)memory[_pop()]);
  // retrieves the column type
  c->type = (byte)_pop();
}

void MastersDBVM::NewResultRecord()
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
