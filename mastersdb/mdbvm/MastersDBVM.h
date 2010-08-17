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
 * 12.08.2010
 *  Added the HALT instruction.
 *  Added the Execute method.
 * 13.08.2010
 *  Added the ADDVAL and INSTBL instructions.
 * 15.08.2010
 *  mdbQueryResult re-factoring.
 *  Added the PUSHOF, NEWCOL, NEWREC instructions.
 *  Added the DSCTBL instruction.
 * 16.08.2010
 *  Added the getCodePointer function.
 */

#ifndef MASTERSDBVM_H_
#define MASTERSDBVM_H_

#include <stdint.h>
#include <vector>

extern "C" {
  #include "../mastersdb.h"
}

using namespace std;

namespace MDB
{

/*
 * MastersDB VM instruction format:
 *
 * IIII IIDD DDDD DDDD
 *
 * I - instruction bits (64 instructions possible)
 * D - data bits (max. value is 1024)
 */

// MastersDB VM instruction macros

#define MVI_OP_MASK     0xFC
#define MVI_DATA_MASK   0x03

#define MVI_OPCODE(b)     (b & MVI_OP_MASK)>>2
#define MVI_DATA(b1,b2)   (((uint16)(b1 & MVI_DATA_MASK))<<8) + b2

#define MVI_ENCODE(i,d)   (i<<2) + (((uint8)(d>>8)) & MVI_DATA_MASK)

class MastersDBVM
{
private:
  static const uint16 MDB_VM_BYTECODE_SIZE = 1024;
  static const uint16 MDB_VM_MEMORY_SIZE = 1024;
  static const uint16 MDB_VM_STACK_SIZE = 64;
  static const uint16 MDB_VM_TABLES_SIZE = 8;

public:
  enum mdbInstruction {
    /*
     * Stack operations
     */
    PUSH,   // PUSH
    POP,    // POP
    PUSHOB, // PUSH OFFSET BYTE
    /*
     * Table operations
     */
    ADDTBL, // ADD TABLE
    ADDCOL, // ADD COLUMN
    ADDVAL, // ADD VALUE
    CRTBL,  // CREATE TABLE
    LDTBL,  // LOAD TABLE
    USETBL, // USE TABLE
    INSTBL, // INSERT INTO TABLE
    DSCTBL, // DESCRIBE TABLE
    /*
     * Result operations
     */
    NEWCOL, // NEW RESULT COLUMN
    NEWREC, // NEW RESULT RECORD
    /*
     * Record (B-tree) operations
     */
    NXTREC, // NEXT RECORD
    NEWRC,  // NEW RECORD
    /*
     * VM Control operations
     */
    HALT
  };

  // A virtual table
  struct mdbVirtualTable
  {
    mdbTable *table;              // used for storing table meta-data
    char *record;                 // used for storing the current record
    mdbBtreeTraversal *traversal; // used for traversing the B-tree
    uint8 cp;                     // column pointer
    char *rp;                     // record pointer
  };

  // The MastersDB Query Language result
  struct mdbQueryResult
  {
    vector<mdbColumnRecord*> *columns;  // columns array
    vector<char*> *records;             // records array
    uint32 record_size;                 // record size
    uint8 cp;                           // column pointer
    char *rp;                           // record pointer
  };

private:

  // MastersDB VM program
  uint8 bytecode[MDB_VM_BYTECODE_SIZE];
  uint16 ip;                  // instruction pointer (current MVI)
  uint16 cp;                  // byte code pointer (when adding instruction)

  // MastersDB VM memory
  char *memory[MDB_VM_MEMORY_SIZE];

  // MastersDB VM stack
  uint16 stack[MDB_VM_STACK_SIZE];
  uint16 sp;                  // stack pointer (top of the stack)

  // table-specific memory
  mdbVirtualTable tables[MDB_VM_TABLES_SIZE];
  uint8 tp;                   // current (virtual) table pointer

  mdbQueryResult result;      // MQL result memory

  mdbDatabase *db;            // MastersDB database (mdbDatabase*)

  uint8 opcode;               // INSTRUCTION part current decoded operation
  uint16 data;                // DATA part of the current decoded operation

  // Resets the MastersDB virtual machine
  void Reset();
  void ClearResult();

public:
  MastersDBVM(mdbDatabase *db);

  uint16 getCodePointer()
  {
    return cp;
  }

  void AddInstruction(uint8 opcode, uint16 data)
  {
    bytecode[cp++] = MVI_ENCODE(opcode,data);
    bytecode[cp++] = (uint8)data;
  };

  void Store(char* data, uint16 ptr)
  {
    memory[ptr] = data;
  };

  void Decode();

  void Execute()
  {
    ClearResult();
    do {
      Decode();
    }
    while (opcode != MastersDBVM::HALT);
  }

  // Stack operations

  /*
   * Pushes value memory[DATA] to stack
   */
  void Push()
  {
    stack[sp++] = *((uint16*)memory[data]);
  }

  /*
   * Pops value from stack and stores it in memory[DATA]
   */
  void Pop()
  {
    memory[data] = (char*)(new uint16);
    *((uint16*)memory[data]) = stack[--sp];
  }

  /*
   * Pops a byte and stores it at (memory[DATA] + offset).
   *
   * Parameters:
   * -----------
   *   DATA           - source
   *   stack[sp-1]    - offset
   */
  void PushOffsetByte()
  {
    uint32 offset = _pop();
    stack[sp++] = *((byte*)(memory[data] + offset));
  }

  /*
   * Silent pop - pops a value from stack and returns it.
   */
  uint16 _pop()
  {
    return stack[--sp];
  }

  // Table operations
  void AddTable();
  void AddColumn();
  void AddValue();
  void CreateTable();
  void LoadTable();
  void InsertIntoTable();
  void DescribeTable();

  /*
   * Sets the value of the current table to DATA.
   */
  void UseTable()
  {
    tp = (uint8)data;
  }

  // Result operations
  void NewResultColumn();
  void NewResultRecord();

  // Record (B-tree) operations
  void NextRecord();
  void NewRecord();

  virtual ~MastersDBVM();
};

}

#endif /* MASTERSDBVM_H_ */
