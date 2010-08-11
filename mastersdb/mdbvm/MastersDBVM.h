/*
 * MastersDBVM.h
 *
 * MastersDB query language interpreter virtual machine
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

#ifndef MASTERSDBVM_H_
#define MASTERSDBVM_H_

#include <stdint.h>

namespace MDB
{

/*
 * MastersDB VM instruction format:
 *
 * IIII IIDD DDDD DDDD
 *
 * I - instruction bits (64 instruction possible)
 * D - data bits (max. value is 1024)
 */

// MastersDB VM instruction macros

#define MVI_OP_MASK     0xFC
#define MVI_DATA_MASK   0x03

#define MVI_OPCODE(b)     (b & MVI_OP_MASK)>>2
#define MVI_DATA(b1,b2)   (((uint16_t)(b1 & MVI_DATA_MASK))<<8) + b2

#define MVI_ENCODE(i,d)   (i<<2) + (((uint8_t)(d>>8)) & MVI_DATA_MASK)

class MastersDBVM
{
private:
  static const uint16_t MDB_VM_BYTECODE_SIZE = 1024;
  static const uint16_t MDB_VM_MEMORY_SIZE = 1024;
  static const uint16_t MDB_VM_STACK_SIZE = 64;
  static const uint16_t MDB_VM_TABLES_SIZE = 8;

public:
  enum mdbInstruction {
    /*
     * Stack operations
     */
    PUSH,   // push value DATA to stack
    POP,    // pop value from stack and store it in memory[DATA]
    /*
     * Table operations
     */
    ADDTBL, // ADD TABLE
    ADDCOL, // ADD COLUMN
    CRTBL,  // CREATE TABLE
    LDTBL,  // LOAD TABLE
    USETBL, // USE TABLE
    /*
     * Record (B-tree) operations
     */
    NEXTRC, // NEXT RECORD
    NEWRC,  // NEW RECORD
  };

  // A virtual table
  struct mdbVirtualTable
  {
    void *table;      // mdbTable* (used for storing table meta-data)
    char *record;     // record storage (used for storing the current record)
    void *traversal;  // mdbBtreeTraversal* (used for traversing the B-tree)
    uint8_t cp;       // column pointer
  };

  // The MastersDB Query Language result
  struct mdbQueryResult
  {
    void *columns;        // mdbColumn* array
    uint8_t num_columns;  // number of columns
    char *records;        // used for storing the query results
    uint32_t num_records; // number of records
    uint32_t record_size; // size of a record
  };

private:

  // MastersDB VM program
  uint8_t bytecode[MDB_VM_BYTECODE_SIZE];
  uint16_t ip;                // instruction pointer (current MVI)
  uint16_t cp;                // byte code pointer (when adding instruction)

  // MastersDB VM memory
  char *memory[MDB_VM_MEMORY_SIZE];

  // MastersDB VM stack
  uint16_t stack[MDB_VM_STACK_SIZE];
  uint16_t sp;                // stack pointer (top of the stack)

  // table-specific memory
  mdbVirtualTable tables[MDB_VM_TABLES_SIZE];
  uint8_t tp;                 // current (virtual) table pointer

  mdbQueryResult result;      // MQL result memory

  void *db;                   // MastersDB database (mdbDatabase*)

  uint8_t opcode;             // current decoded operation
  uint16_t data;              // DATA part of the current decoded operation

  // Resets the MastersDB virtual machine
  void Reset();
  void ClearResult();

public:
  MastersDBVM(void *db);

  void AddInstruction(uint8_t opcode, uint16_t data)
  {
    bytecode[cp++] = MVI_ENCODE(opcode,data);
    bytecode[cp++] = (uint8_t)data;
  };

  void Store(char* data, uint16_t ptr)
  {
    memory[ptr] = data;
  };

  void Decode();

  // Stack operations

  /*
   * Pushes value DATA to stack
   */
  void Push()
  {
    stack[sp++] = data;
  }

  /*
   * Pops value from stack and stores it in memory[DATA]
   */
  void Pop()
  {
    memory[data] = (char*)(new uint16_t);
    *((uint16_t*)memory[data]) = stack[--sp];
  }

  // Table operations
  void AddTable();
  void AddColumn();
  void CreateTable();
  void LoadTable();

  /*
   * Sets the value of the current table to DATA.
   */
  void UseTable()
  {
    tp = (uint8_t)data;
  }

  // Record (B-tree) operations
  void NextRecord();
  void NewRecord();

  virtual ~MastersDBVM();
};

}

#endif /* MASTERSDBVM_H_ */
