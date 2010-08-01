/*
 * MQLCondition.cpp
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

#include "MQLCondition.h"

#include <cstdlib>

MQLCondition::MQLCondition(uint8 left,
    MQLConditionType cond, void* right, MQLType rightType)
{
  this->left = left;
  this->rightType = rightType;
  this->cond = cond;

  switch (rightType) {
    case COLUMN:
      this->right = *((uint8*)right);
      break;
    case NUMBER:
      this->iRight = atoi(((string*)right)->c_str());
      break;
    case STRING:
      this->sRight = ((string*)right)->substr(1,((string*)right)->length() - 1);
      break;
    default:
      break;
  }
}

MQLConditionType MQLCondition::getType()
{
  return this->cond;
}

uint8 MQLCondition::getLeft()
{
  return this->left;
}

string *MQLCondition::getRightString()
{
  return &sRight;
}

uint8 MQLCondition::getRight()
{
  return right;
}

uint32 MQLCondition::getRightInteger()
{
  return iRight;
}

MQLType MQLCondition::getRightType()
{
  return this->rightType;
}
