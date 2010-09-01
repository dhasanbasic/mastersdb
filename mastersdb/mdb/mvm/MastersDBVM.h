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
 *  Added the INSVAL and INSREC instructions.
 * 15.08.2010
 *  mdbQueryResult re-factoring.
 *  Added the PUSHOF, NEWCOL, NEWREC instructions.
 *  Added the DSCTBL instruction.
 * 16.08.2010
 *  Added the getCodePointer method.
 * 17.08.2010
 *  Added the RewriteInstruction method.
 *  Change the byte-code container to be a 16-bit array.
 *  Added new instructions: NOP, CPYCOL, NXTREC, CPYREC, JMP, JMPF.
 * 18.08.2010
 *  The mdbVirtualTable structure now contains:
 *    - a map of its column names,
 *    - a vector of the column value positions in the source record.
 *  Added a new instruction: CPYVAL.
 * 19.08.2010
 *  Added a new instruction: NEWREC.
 * 21.08.2010
 *  The results from executions are now returned by Execute().
 */

#ifndef MASTERSDBVM_H_
#define MASTERSDBVM_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <map>

extern "C" {
  #include "../mdb.h"
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

// Column map typedefs
typedef map<string,uint16>        mdbCMap;
typedef pair<string,uint16>       mdbCMapPair;
typedef mdbCMap::iterator         mdbCMapIter;
typedef pair<mdbCMapIter,bool>    mdbCMapResult;

// MastersDB VM instruction macros

#define MVI_OP_MASK     0xFC00
#define MVI_DATA_MASK   0x03FF

#define MVI_OPCODE(dw)    ((dw & MVI_OP_MASK)>>10)
#define MVI_DATA(dw)      (dw & MVI_DATA_MASK)

#define MVI_ENCODE(i,d)   (((uint16)i)<<10)+(d & MVI_DATA_MASK)

class MastersDBVM
{
private:
  static const uint16 MDB_VM_BYTECODE_SIZE = 1024;
  static const uint16 MDB_VM_MEMORY_SIZE = 1024;
  static const uint16 MDB_VM_STACK_SIZE = 64;
  static const uint16 MDB_VM_TABLES_SIZE = 8;

public:
  enum mdbInstruction {
    // No operation
    NOP,
    /*
     * Stack operations
     */
    PUSHM,  // PUSH MEMORY (memory to stack)
    POPM,   // POP MEMORY (stack to memory)
    /*
     * Table operations
     */
    NEWTBL, // NEW TABLE (allocate new virtual table)
    CRTTBL, // CREATE TABLE
    LDTBL,  // LOAD TABLE
    SETTBL, // SET TABLE
    DSCTBL, // DESCRIBE TABLE
    /*
     * Column operations
     */
    NEWCOL, // NEW COLUMN
    CPYCOL, // COPY COLUMN (to result columns)
    /*
     * Table record operations
     */
    INSVAL, // INSERT VALUE
    INSREC, // INSERT RECORD
    NXTREC, // NEXT RECORD
    CPYREC, // COPY RECORD (to result store)
    CPYVAL, // COPY VALUE (to result store)
    NEWREC, // NEW RESULT RECORD
    /*
     * VM Control operations
     */
    JMP,    // JMP to DATA
    JMPF,   // JMP ON FAILURE (_pop() == MVI_FAILURE)
    HALT,   // HALT (clear VM memory and stop execution)
  };

  // MastersDB Virtual Machine instruction constants
  enum mdbMVIConstants
  {
    MVI_NOP,
    MVI_SUCCESS,
    MVI_FAILURE,
  };

  // A virtual table
  struct mdbVirtualTable
  {
    mdbTable *table;              // used for storing table meta-data
    mdbBtreeTraversal *traversal; // used for traversing the B-tree
    char *record;                 // used for storing the current record
    uint32 record_size;           // size of a record
    uint8 cp;                     // current column
    vector<uint32> vals;          // column value positions in the record
    mdbCMap cols;                 // used for mapping column names to
                                  // table->columns[] indexes
  };

  // The MastersDB Query Language result
  struct mdbQueryResult
  {
    vector<mdbColumnRecord*> columns; // columns array
    vector<char*> records;            // records array
    char *record;                     // current result record
    uint32 record_size;               // record size
    uint8 cp;                         // current column
    vector<uint32> vals;              // column value positions in the record
  };

private:

  // MastersDB VM program
  uint16 bytecode[MDB_VM_BYTECODE_SIZE];
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

  mdbQueryResult *result;     // MQL result memory

  mdbDatabase *db;            // MastersDB database (mdbDatabase*)

  uint8 opcode;               // current decoded operation code
  uint16 data;                // current decoded data

  // Stack operations

  /*
   * Pushes value memory[DATA] to stack
   */
  void PushMemory()
  {
    stack[sp++] = *((uint16*)memory[data]);
  }

  /*
   * Pops value from stack and stores it in memory[DATA]
   */
  void PopMemory()
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
   * Silent push - pushes a value to stack.
   */
  void _push(uint16 value)
  {
    stack[sp++] = value;
  }

  /*
   * Silent pop - pops a value from stack and returns it.
   */
  uint16 _pop()
  {
    return stack[--sp];
  }

  // Table operations
  void NewTable();
  void CreateTable();
  void LoadTable();
  void DescribeTable();

  void SetTable()
  {
    tp = (uint8)data;
  }

  // Column operations
  void NewColumn();
  void CopyColumn();

  // Source/Destination record operations
  void InsertValue();
  void InsertRecord();
  void NextRecord();
  void CopyRecord();
  void CopyValue();
  void NewRecord();

  // VM operations
  void Reset();
  void Decode();

  void Jump()
  {
    ip = data;
  }

  /*
   * Jumps if the stack contains an MVI_FAILURE value
   */
  void JumpOnFailure()
  {
    if (_pop() == MVI_FAILURE)
    {
      ip = data;
    }
  }

public:
  MastersDBVM();

  void setDatabase(mdbDatabase *db)
  {
    this->db = db;
  }

  uint16 getCodePointer()
  {
    return cp;
  }

  void AddInstruction(uint8 opcode, uint16 data)
  {
    bytecode[cp++] = MVI_ENCODE(opcode,data);
  };

  void RewriteInstruction(uint16 cptr, uint8 opcode, uint16 data)
  {
    bytecode[cptr] = MVI_ENCODE(opcode,data);
  }

  void StoreData(char* data, uint16 ptr)
  {
    memory[ptr] = data;
  };

  mdbQueryResult* Execute()
  {
    mdbQueryResult *res = NULL;
    do {
      Decode();
    }
    while (opcode != MastersDBVM::HALT);

    if (result->columns.size() > 0)
    {
      res = result;
      result = new mdbQueryResult;
      result->cp = 0;
      result->record = NULL;
      result->record_size = 0;
      result->vals.push_back(0);
    }
    return res;
  }

  virtual ~MastersDBVM();
};

}

#endif /* MASTERSDBVM_H_ */
