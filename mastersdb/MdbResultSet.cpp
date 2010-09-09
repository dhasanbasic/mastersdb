/*
 * MdbResultSet.cpp
 *
 * MastersDB result record set
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
 * 21.08.2010
 *  Initial version of file.
 * 09.09.2010
 *  Implemented GetColumnCount, GetColumnName, GetColumnType methods.
 */

#include "MastersDB.h"
#include "mdb/mvm/mdbVirtualMachine.h"

using namespace MDB;

namespace MastersDB
{

void MdbResultSet::Close()
{
  if (rs != NULL)
  {
    delete (mdbQueryResults*)rs;
    rs = NULL;
  }
}

uint32_t MdbResultSet::GetRecordCount()
{
  if (rs != NULL)
  {
    return ((mdbQueryResults*)rs)->GetRecordCount();
  }
  return 0;
}

bool MdbResultSet::ToFirst()
{
  if (rs != NULL)
  {
    r = 0;
    return true;
  }
  return false;
}

bool MdbResultSet::ToNext()
{
  if (rs != NULL)
  {
    if (r < (((mdbQueryResults*)rs)->GetRecordCount() - 1))
    {
      r++;
      return true;
    }
  }
  return false;
}

bool MdbResultSet::ToPrevious()
{
  if (rs != NULL)
  {
    if (r > 0)
    {
      r--;
      return true;
    }
  }
  return false;
}

bool MdbResultSet::ToLast()
{
  if (rs != NULL)
  {
    r = ((mdbQueryResults*)rs)->GetRecordCount() - 1;
    return true;
  }
  return false;
}

uint8_t MdbResultSet::GetColumnCount()
{
  return ((mdbQueryResults*)rs)->getColumnCount();
}

std::string MdbResultSet::GetColumnName(uint8_t column)
{
  char *name;
  name = ((mdbQueryResults*)rs)->getColumn(column)->name;
  return std::string(name + 4, *((uint32*)name));
}

MdbDatatypes MdbResultSet::GetColumnType(uint8_t column)
{
  return (MdbDatatypes)((mdbQueryResults*)rs)->getColumn(column)->type;
}

MdbDatatypes MdbResultSet::GetColumnType(std::string column)
{
  char name[column.length() + 4];
  *((uint32*)name) = column.length();
  column.copy(name + 4, column.length());
  return (MdbDatatypes)((mdbQueryResults*)rs)->getColumn(name)->type;
}

int32_t MdbResultSet::GetIntValue(uint8_t column)
{
  mdbQueryResults *results;
  mdbDatatype *type;
  int32_t val = 0;

  if (rs != NULL)
  {
    results = (mdbQueryResults*)rs;
    if (column < results->getColumnCount())
    {
      type = ((mdbDatabase*)DB)->datatypes +
          results->getColumn(column)->type;
      memcpy(&val, results->GetRecord(r) +
          results->getColumnOffset(column), type->size);
    }
  }
  return val;
}

int32_t MdbResultSet::GetIntValue(string column)
{
//  mdbQueryResult *result;
//  uint8_t c;
//  if (rs != NULL)
//  {
//    result = (mdbQueryResult*)rs;
//    if (result->cols.find(column) != result->cols.end())
//    {
//      c = result->cols[column].second;
//      return *((int32_t*)(result->records[r] + result->vals[c]));
//    }
//  }
  return 0;
}

string MdbResultSet::GetStringValue(uint8_t column)
{
  mdbQueryResults *results;
  char *ret;
  if (rs != NULL)
  {
    results = (mdbQueryResults*)rs;
    if (column < results->getColumnCount())
    {
      ret = (results->GetRecord(r) + results->getColumnOffset(column));
      return string(ret + 4, *((uint32_t*)ret));
    }
  }
  return string("");
}

string MdbResultSet::GetStringValue(string column)
{
//  mdbQueryResult *result;
//  char *ret;
//  uint8_t c;
//  if (rs != NULL)
//  {
//    result = (mdbQueryResult*)rs;
//    if (result->cols.find(column) != result->cols.end())
//    {
//      c = result->cols[column].second;
//      ret = result->records[r] + result->vals[c];
//      return string(ret + 4, *((uint32_t*)ret));
//    }
//  }
  return string("");
}

}
