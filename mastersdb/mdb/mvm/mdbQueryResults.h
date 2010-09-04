/*
 * mdbQueryResults.h
 *
 * MastersDB query processing results
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
 * 03.09.2010
 *  Initial version of file.
 */

#ifndef MDBQUERYRESULTS_H_
#define MDBQUERYRESULTS_H_

#include "mdbVirtualTable.h"

namespace MDB
{

class mdbQueryResults: public MDB::mdbVirtualTable
{
private:
  vector<char*> records;
public:
  mdbQueryResults(mdbDatabase *db) : mdbVirtualTable(db) { };
  uint32 GetRecordCount();
  uint32 GetRecordSize();
  void NewRecord();
  char* GetRecord(uint32 r);
  virtual ~mdbQueryResults();
};

}

#endif /* MDBQUERYRESULTS_H_ */
