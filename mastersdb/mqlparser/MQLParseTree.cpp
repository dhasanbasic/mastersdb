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

#include "MQLParseTree.h"

MQLParseTree::MQLParseTree()
{
}

uint8 MQLParseTree::addTable(string table)
{
  pair<map<string,uint8>::iterator,bool> ret;
  ret = tables.insert(pair<string,uint8>(table,tables.size()));
  return ret.first->second;
}

uint8 MQLParseTree::getTableID(string table)
{
  map<string,uint8>::iterator it;
  it = tables.find(table);
  return it->second;
}

void MQLParseTree::setStatement(MQLStatement *statement)
{
  this->statement = statement;
}

uint8 MQLParseTree::addColumn(string column)
{
  pair<map<string,uint8>::iterator,bool> ret;
  ret = columns.insert(pair<string,uint8>(column,columns.size()));
  return ret.first->second;
}

uint8 MQLParseTree::getColumnID(string column)
{
  map<string,uint8>::iterator it;
  it = columns.find(column);
  return it->second;
}

//MQLParseTree::~MQLParseTree()
//{
//}

