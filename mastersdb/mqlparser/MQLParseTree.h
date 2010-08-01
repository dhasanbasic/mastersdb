/*
 * MQLParseTree.h
 *
 * MastersDB query language tree built upon an MQL statement
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
 * 01.08.2010
 *  Initial version of file.
 */

#ifndef MQLParseTree_H_
#define MQLParseTree_H_

#include "../common.h"
#include "MQLStatement.h"

#include <string>
#include <vector>
#include <map>

using namespace std;

class MQLParseTree
{
private:
  map<string, uint8> tables;    // hashed table names
  map<string, uint8> columns;   // hashed column names
  MQLStatement *statement;      // MQL statement in parsed form
public:
  MQLParseTree();
  uint8 addTable(string table);
  uint8 getTableID(string table);
  uint8 addColumn(string column);
  uint8 getColumnID(string column);
  void setStatement(MQLStatement *statement);
//  virtual ~MQLParseTree();
};

#endif /* MQLParseTree_H_ */
