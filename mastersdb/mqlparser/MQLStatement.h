/*
 * MQLStatement.h
 *
 * MastersDB query language statement
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

#ifndef MQLSTATEMENT_H_
#define MQLSTATEMENT_H_

#include "../common.h"
#include "MQLCondition.h"

#include <vector>

using namespace std;

enum MQLStatementType {
  SELECT, UPDATE, INSERT
};

class MQLStatement
{
private:
  vector<uint8> columns;
  vector<uint8> tables;
  vector<MQLCondition*> conditions;
  MQLStatementType type;
public:
  MQLStatement(MQLStatementType type);
  MQLStatementType getType();
  vector<uint8> *getColumns();
  vector<uint8> *getTables();
  vector<MQLCondition*> *getConditions();
  void addColumn(uint8 n);
  void addTable(uint8 n);
  void addCondition(MQLCondition* condition);
  virtual ~MQLStatement();
};

#endif /* MQLSTATEMENT_H_ */
