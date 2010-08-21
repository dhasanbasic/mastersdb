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
 */

#include "MastersDB.h"
#include "mdb/mvm/MastersDBVM.h"

using namespace MDB;

typedef MastersDBVM::mdbQueryResult mdbQueryResult;

namespace MastersDB
{

void MdbResultSet::Close()
{
  uint32_t i;
  mdbQueryResult *result;
  if (rs != NULL)
  {
    result = (mdbQueryResult*)rs;
    if (result->records.size() > 0)
    {
      for (i = 0; i < result->columns.size(); i++)
      {
        delete result->columns[i];
      }
      for (i = 0; i < result->records.size(); i++)
      {
        delete[] result->records[i];
      }
      delete[] result->record;
    }
    delete result;
    rs = NULL;
  }
}

uint32_t MdbResultSet::GetRecordCount()
{
  if (rs != NULL)
  {
    return ((mdbQueryResult*)rs)->records.size();
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
    if (r < (((mdbQueryResult*)rs)->records.size() - 1))
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
    r = ((mdbQueryResult*)rs)->records.size() - 1;
    return true;
  }
  return false;
}

int32_t MdbResultSet::GetIntValue(uint8_t column)
{
  mdbQueryResult *result;
  if (rs != NULL)
  {
    result = (mdbQueryResult*)rs;
    if (column < result->columns.size())
    {
      return *((int32_t*)(result->records[r] + result->vals[column]));
    }
  }
  return 0;
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
  mdbQueryResult *result;
  char *ret;
  if (rs != NULL)
  {
    result = (mdbQueryResult*)rs;
    if (column < result->columns.size())
    {
      ret = (result->records[r] + result->vals[column]);
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
