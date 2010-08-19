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
 */

#ifndef MQLSELECT_H_
#define MQLSELECT_H_

#include "../mdbvm/MastersDBVM.h"

using namespace std;

namespace MDB
{
// Table map typedefs
typedef pair<uint16,mdbCMap*>     mdbTMapItem;
typedef map<string,mdbTMapItem>   mdbTMap;
typedef pair<string,mdbTMapItem>  mdbTMapPair;
typedef mdbTMap::iterator         mdbTMapIter;
typedef pair<mdbTMapIter,bool>    mdbTMapResult;

// Destination columns
typedef pair<string,string>       mdbDestinationColumn;

class MQLSelect
{
private:
  string MDB_DEFAULT;
  vector<mdbDestinationColumn> destColumns;
  mdbTMap tables;
  MastersDBVM *VM;
  uint16 dptr;

  void GenSingleTableSelect();

public:
  MQLSelect();
  bool MapColumn(string *column, string *table, uint16 dp);
  void MapTable(string *table, uint16 tp);
  void Reset();
  void GenerateBytecode();

  void setDataPointer(uint16 dptr)
  {
    this->dptr = dptr;
  }

  void setVM(MastersDBVM *vm)
  {
    VM = vm;
  }

  virtual ~MQLSelect();
};

}

#endif /* MQLSELECT_H_ */
