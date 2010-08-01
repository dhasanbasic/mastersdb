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

MQLCondition::MQLCondition(uint8 left,
    MQLConditionType comparison, void *right, MQLType rightType)
{
  this->comparison = comparison;
  this->left = left;
  this->right = right;
  this->rightType = rightType;
}

MQLConditionType MQLCondition::getComparison()
{
  return this->comparison;
}

uint8 MQLCondition::getLeftOperand()
{
  return this->left;
}

void *MQLCondition::getRightOperand()
{
  return this->right;
}

MQLType MQLCondition::getRightType()
{
  return this->rightType;
}
