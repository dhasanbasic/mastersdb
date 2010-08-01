/*
 * MQLCondition.h
 *
 * MastersDB query language WHERE parse tree
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

#ifndef MQLCONDITION_H_
#define MQLCONDITION_H_

#include "../common.h"

using namespace std;

enum MQLConditionType {
  Less, LessOrEqual, Equal, GreaterOrEqual, Greater
};

enum MQLType {
  Number, String, Column
};

class MQLCondition
{
private:
  uint8 left;              // left operand
  void* right;              // right operand
  MQLConditionType comparison; // type of comparison
  MQLType rightType;        // data type of the right argument
public:
  MQLCondition(uint8 left,
      MQLConditionType comparison, void* right, MQLType rightType);

  MQLConditionType getComparison();
  uint8 getLeftOperand();
  void* getRightOperand();
  MQLType getRightType();
};

#endif /* MQLCONDITION_H_ */
