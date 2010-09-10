/*
 * MQLSelect.h
 *
 * MastersDB Query Language SELECT statement (abstract syntax tree)
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
 * 16.08.2010
 *  Initial version of file.
 * 18.08.2010
 *  Moved mdbCMap type definitions to MastersDBVM.h.
 *  No more need for the allColumns flag.
 *  GenSingleTableSelect() re-factoring.
 * 04.09.2010
 *  Added mdbCondition structure.
 * 08.09.2010
 *  Added support for processing cross table joins.
 */

#ifndef MQLSELECT_H_
#define MQLSELECT_H_

#include "../mvm/mdbVirtualMachine.h"

#include <set>

using namespace std;

namespace MDB
{

struct mdbOperation
{
  mdbOperation *left_child;         // left child
  mdbOperation *right_child;        // right child
  mdbOperationType type;            // type of comparison operation
  uint32 param;                     // parameter (see below)
  uint16 param_addr;                // virtual machine memory address
                                    // where "param" has been stored

  /* parameter value specification
   * ---------------------------
   *  VTTT AAAA BBBB XXXX XXXX XXZZ ZZ ZZZZ
   * ---------------------------
   *  V : 0 - right operand is a column value
   *      1 - right operand is a direct value
   *  T : operation type (mdbOperationType)
   *  A : left operand virtual table index
   *  B : right operand virtual table index (if column value)
   *  X : left operand virtual machine memory address (column name)
   *  Y : right operand virtual machine memory address (column name
   *      (or direct value)
   */
};

// Table map typedefs
struct mdbTableInfo
{
  uint8 tp;
  uint16 dp;
  uint16 cdp;
  mdbColumnMap columns;
};

typedef map<string,mdbTableInfo*>   mdbTableMap;
typedef pair<string,mdbTableInfo*>  mdbTableMapPair;
typedef mdbTableMap::iterator         mdbTableMapIterator;
typedef pair<mdbTableMapIterator,bool>    mdbTableMapResult;

// Destination columns
typedef pair<string,string>       mdbDestinationColumn;

class MQLSelect
{
private:
  string MDB_DEFAULT;
  vector<mdbDestinationColumn> destColumns;
  mdbTableMap tables;
  mdbVirtualMachine *VM;
  uint16 dptr;
  mdbOperation *where;
  set<uint8> joins;

  uint16 loop_start[mdbVirtualMachine::MDB_VM_TABLES_SIZE];

  void GenLoadTables();
  void GenDefineResults(bool &asterisk);
  void GenTableLoop(uint16 tp);
  void GenCopyResult(bool asterisk);
  void GenConditionCheck(uint16 fail_address);

public:
  MQLSelect();

  void MapColumn(
      string *column,
      string *table,
      uint16 &dp,
      mdbTableInfo* &ti,
      bool destination = true);

  void ResolveDefaultTable(string *table, uint16& dp);
  void Reset();
  void GenerateBytecode();

  void setDataPointer(uint16 dptr)
  {
    this->dptr = dptr;
  }

  void setOperation(mdbOperation *oper)
  {
    where = oper;
  }

  void setVM(mdbVirtualMachine *vm)
  {
    VM = vm;
  }

  void addJoin(uint8 table)
  {
    joins.insert(table);
  }

  virtual ~MQLSelect();
};

}

#endif /* MQLSELECT_H_ */
