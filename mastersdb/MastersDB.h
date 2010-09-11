/*
 * MastersDB.h
 *
 * MastersDB C++ Library
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
 *  Added GetColumnCount, GetColumnName, GetColumnType methods.
 */


#ifndef MASTERSDB_H_
#define MASTERSDB_H_

#include <stdint.h>
#include <string>

namespace MastersDB
{

// MastersDB data types
enum MdbDatatypes
{
  MDB_INT_8,
  MDB_INT_16,
  MDB_INT_32,
  MDB_FLOAT,
  MDB_STRING
};

// forward declarations of the MastersDB classes
class MdbDatabase;
class MdbResultSet;

class MdbDatabase
{
private:
  void *VM;                 // internal pointer to MDB Virtual Machine
  void *DB;                 // internal pointer to database
  void *S;                  // internal pointer to MQL SELECT AST
  void *P;                  // internal pointer to MQL Parser

  // prevent explicit object construction;
  MdbDatabase() : VM(0),DB(0),S(0),P(0) {}
  MdbDatabase(const MdbDatabase &);
  MdbDatabase & operator=(const MdbDatabase &);

public:
  virtual ~MdbDatabase() { Close(); }
  static MdbDatabase* CreateDatabase(std::string filename);
  static MdbDatabase* OpenDatabase(std::string filename);
  MdbResultSet* ExecuteMQL(std::string statement);
  std::string ExplainMQL(std::string statement);
  void Close();
};

class MdbResultSet
{
private:
  void *DB;                 // internal pointer to database

  // MdbDatabase can construct objects of this class
  friend class MdbDatabase;

  void *rs;
  uint32_t r;

  // prevent explicit object construction;
  MdbResultSet() : rs(0),r(0) {}
  MdbResultSet(const MdbResultSet &);
  MdbResultSet & operator=(const MdbResultSet &);

public:
  virtual ~MdbResultSet() { Close(); }
  void Close();
  uint32_t GetRecordCount();

  bool ToFirst();
  bool ToNext();
  bool ToPrevious();
  bool ToLast();

  uint8_t GetColumnCount();
  std::string GetColumnName(uint8_t column);
  MdbDatatypes GetColumnType(uint8_t column);
  MdbDatatypes GetColumnType(std::string column);

  int32_t GetIntValue(uint8_t column);
  int32_t GetIntValue(std::string column);
  std::string GetStringValue(uint8_t column);
  std::string GetStringValue(std::string column);
};

}

#endif /* MASTERSDB_H_ */
