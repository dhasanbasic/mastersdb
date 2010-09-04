/*
 * mdbQueryResults.cpp
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

#include "mdbQueryResults.h"

namespace MDB
{

uint32 mdbQueryResults::GetRecordCount()
{
  return records.size();
}

uint32 mdbQueryResults::GetRecordSize()
{
  return record_size;
}

void mdbQueryResults::NewRecord()
{
  if (record != NULL)
  {
    records.push_back(record);
  }
  record = new char[record_size];
}

char* mdbQueryResults::GetRecord(uint32 r)
{
  return records[r];
}

mdbQueryResults::~mdbQueryResults()
{
  if (records.size() > 0)
  {
    for (uint32 r = 0; r < records.size(); r++)
    {
      delete[] records[r];
    }
  }
}

}
