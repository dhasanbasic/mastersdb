/*
 * MQLStatement.cpp
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

#include "MQLStatement.h"

MQLStatement::MQLStatement(MQLStatementType type)
{
  this->type = type;
}

vector<uint8> *MQLStatement::getColumns()
{
  return &this->columns;
}

MQLStatementType MQLStatement::getType()
{
  return this->type;
}

vector<MQLCondition*> *MQLStatement::getConditions()
{
  return &this->conditions;
}

vector<uint8> *MQLStatement::getTables()
{
  return &this->tables;
}

void MQLStatement::addColumn(uint8 n)
{
  columns.push_back(n);
}

void MQLStatement::addTable(uint8 n)
{
  tables.push_back(n);
}

void MQLStatement::addCondition(MQLCondition *condition)
{
  conditions.push_back(condition);
}

MQLStatement::~MQLStatement()
{
  uint8 i;
  for (i=0; i < conditions.size(); i++) {
    delete conditions[i];
  }
}
