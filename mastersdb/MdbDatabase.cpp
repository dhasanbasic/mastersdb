/*
 * MdbDatabase.cpp
 *
 * DESCRIPTION
 *
 * Copyright (C) 2010, Dinko Hasanbasic (dinko.hasanbasic@gmail.com)
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
 * 20.08.2010
 *  Initial version of file.
 */

#include "MastersDB.h"
#include "mdb/mql/Parser.h"

#include <iostream>

using namespace MDB;

namespace MastersDB
{

MdbDatabase* MdbDatabase::CreateDatabase(string filename)
{
  mdbVirtualMachine *vm;
  MdbDatabase *db;
  MQLSelect *s;
  Parser *p;
  int ret;

  // create the database
  db = new MdbDatabase();
  ret = mdbCreateDatabase((mdbDatabase**)&db->DB, filename.c_str());

  // create a new Parser and MQLSelect AST
  vm = new mdbVirtualMachine((mdbDatabase*)db->DB);
  s = new MQLSelect();
  s->setVM(vm);
  p = new Parser();
  p->setVM(vm);
  p->setSelect(s);

  db->VM = vm;
  db->P = p;
  db->S = s;

  return db;
}

MdbDatabase* MdbDatabase::OpenDatabase(string filename)
{
  mdbVirtualMachine *vm;
  MdbDatabase *db;
  MQLSelect *s;
  Parser *p;
  int ret;

  // create the database
  db = new MdbDatabase();
  ret = mdbOpenDatabase((mdbDatabase**)&db->DB, filename.c_str());

  // create a new Parser and MQLSelect AST
  vm = new mdbVirtualMachine((mdbDatabase*)db->DB);
  s = new MQLSelect();
  s->setVM(vm);
  p = new Parser();
  p->setVM(vm);
  p->setSelect(s);

  db->VM = vm;
  db->P = p;
  db->S = s;

  return db;
}

MdbResultSet* MdbDatabase::ExecuteMQL(string statement)
{
  mdbVirtualMachine *vm = (mdbVirtualMachine*)VM;
  Parser *p = (Parser*)P;
  mdbQueryResults *rs;
  MdbResultSet *mrs;
  string s;
  if (vm != NULL)
  {
    p->Parse((uint8_t*) statement.c_str(), statement.length());
    if ((rs = vm->Execute()) != NULL)
    {
      mrs = new MdbResultSet();
      mrs->rs = rs;
      mrs->DB = DB;
      return mrs;
    }
  }
  return NULL;
}

std::string MdbDatabase::ExplainMQL(string statement)
{
  mdbVirtualMachine *vm = (mdbVirtualMachine*)VM;
  Parser *p = (Parser*)P;
  mdbQueryResults *rs;
  MdbResultSet *mrs;
  string s;
  if (vm != NULL)
  {
    p->Parse((uint8_t*) statement.c_str(), statement.length());
    s.append("Statement:\n");
    s.append(statement);
    s.append("\n");
    s.append(vm->generateVMsnapshot());
    return s;
  }
  return "";
}

void MdbDatabase::Close()
{
  int ret;
  if (DB != NULL)
  {
    ret = mdbCloseDatabase((mdbDatabase*) DB);
    DB = NULL;
    delete (Parser*) P;
    delete (MQLSelect*) S;
    delete (mdbVirtualMachine*) VM;
  }
}

}
