/*
 * MQLSelect.cpp
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

#include "MQLSelect.h"

namespace MDB
{

MQLSelect::MQLSelect()
{
  MDB_DEFAULT = ".Default";
  allColumns = false;
}

/*
 * Sets the allTables flag
 */
void MQLSelect::UseAllColumns()
{
  allColumns = true;
}

/*
 * Maps a column identifier to a data pointer.
 */
bool MQLSelect::MapColumn(string *column, string *table, uint16 dp)
{
  string cTable = (table == NULL) ? MDB_DEFAULT : (*table);
  string cColumn = *column;
  mdbCMap *columns;

  mdbTMapIter t;
  mdbCMapResult c;

  // if the table name is encountered for the first time
  if ((t = tables.find(cTable)) == tables.end())
  {
    // create a new column map
    columns = new mdbCMap();
    (*columns)[cColumn] = dp;
    tables[cTable] = mdbTMapItem(0, columns);
  }
  // otherwise, insert the column name in the appropriate column map
  else
  {
    columns = t->second.second;
    c = columns->insert(mdbCMapPair(cColumn, dp));
    destColumns.push_back(mdbDestinationColumn(cTable, cColumn));
    return (c.second);
  }

  destColumns.push_back(mdbDestinationColumn(cTable, cColumn));

  return true;
}

/*
 * Maps a table identifier to a data pointer.
 */
void MQLSelect::MapTable(string *table, uint16 tp)
{
  mdbTMapItem ti;
  mdbTMapIter iter;
  uint16 i;
  string cTable = *table;
  if (allColumns)
  {
    tables[cTable] = mdbTMapItem(tp, NULL);
    for (i = 0; i < destColumns.size(); i++)
    {
      destColumns[i].first = cTable;
    }
  }
  else
  {
    if (tables.size() == 1)
    {
      ti = tables[MDB_DEFAULT];
      ti.first = tp;
      tables[cTable] = ti;
      tables.erase(MDB_DEFAULT);
      for (i = 0; i < destColumns.size(); i++)
      {
        if (destColumns[i].first == MDB_DEFAULT)
        {
          destColumns[i].first = cTable;
        }
      }
    }
    else
    {
      iter = tables.find(cTable);
      iter->second.first = tp;
    }
  }
}

void MQLSelect::Reset()
{
  mdbTMapIter t;

  for (t = tables.begin(); t != tables.end(); t++)
  {
    delete t->second.second;
  }
  tables.clear();
  destColumns.clear();
  allColumns = false;
}

MQLSelect::~MQLSelect()
{
  if (tables.size() > 0)
  {
    Reset();
  }
}

}
