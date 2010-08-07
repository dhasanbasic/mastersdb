/*
 * MQLParser.h
 *
 * MastersDB Query Language (MQL) parser
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
 * 07.08.2010
 *  Initial version of file.
 */

#ifndef MQLPARSER_H_
#define MQLPARSER_H_

#include <map>
#include <string>

using namespace std;

namespace MDB
{

typedef map<string*, void*> metaMap;
typedef pair<string*, void*> metaPair;

class MQLParser
{
private:
  void *db;                       // pointer to current loaded database
  metaMap tables;     // all tables
  metaMap columns;    // all columns
//  map<string*, void*> indexes;    // all indexes
  void clearMetadata();

  // Disallow copying and empty initialization
  MQLParser(const MQLParser &p);
  MQLParser& operator=(const MQLParser &p);
public:
  MQLParser(void *database);
  void mapMetadata();
  virtual ~MQLParser();
};

}

#endif /* MQLPARSER_H_ */
