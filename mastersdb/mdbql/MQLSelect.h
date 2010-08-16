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
 */

#ifndef MQLSELECT_H_
#define MQLSELECT_H_

#include "../mdbvm/MastersDBVM.h"
#include <vector>
#include <string>
#include <map>

using namespace std;

// Column map typedefs
typedef map<string,uint16>        mdbCMap;
typedef pair<string,uint16>       mdbCMapPair;
typedef mdbCMap::iterator         mdbCMapIter;
typedef pair<mdbCMapIter,bool>    mdbCMapResult;

// Table map typedefs
typedef pair<uint16,mdbCMap*>     mdbTMapItem;
typedef map<string,mdbTMapItem>   mdbTMap;
typedef pair<string,mdbTMapItem>  mdbTMapPair;
typedef mdbTMap::iterator         mdbTMapIter;
typedef pair<mdbTMapIter,bool>    mdbTMapResult;

// Destination columns
typedef pair<string,string>       mdbDestinationColumn;

namespace MDB
{

class MQLSelect
{
private:
  string MDB_DEFAULT;
  vector<mdbDestinationColumn> destColumns;
  mdbTMap tables;
  bool allColumns;
public:
  MQLSelect();
  void UseAllColumns();
  bool MapColumn(string *column, string *table, uint16 dp);
  void MapTable(string *table, uint16 tp);
  void RemapColumns();
  void Reset();
  virtual ~MQLSelect();

  void toString()
  {
    uint16 i;
    mdbTMapIter t;
    mdbCMapIter c;
    for (t = tables.begin(); t != tables.end(); t++)
    {
      printf("%s(%u)\n", t->first.c_str(), t->second.first);

      if (t->second.second != NULL)
      {
        for (c = t->second.second->begin(); c != t->second.second->end(); c++)
        {
          printf("-> %s(%u)\n", c->first.c_str(), c->second);
        }
      }
    }

    printf("ORDER:");
    for (i = 0; i < destColumns.size(); i++)
    {
      printf(" %s.%s", destColumns[i].first.c_str(), destColumns[i].second.c_str());
    }
    printf("\n\n");
  }

};

}

#endif /* MQLSELECT_H_ */
