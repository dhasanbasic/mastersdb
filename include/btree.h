
/*
 * btree.h
 *
 * B-tree structures and function prototypes
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
 * 28.02.2010
 *    Initial version of file.
 * 02.03.2010
 *  Implemented B-tree search.
 * 07.03.2010
 *  Implemented B-tree insertion.
 */

#ifndef BTREE_H_INCLUDED
#define BTREE_H_INCLUDED

#include "types.h"
#include <string.h>

/* B-tree node structure */
typedef struct {
  byte    *data;            /* all data of the node                   */
  uint16  *record_count;    /* pointer to count of records            */
  uint16  *is_leaf;         /* pointer to leaf/internal information   */
  ulong   *children;        /* pointer to child pointer array         */
  byte    *records;         /* pointer to records                     */
} BtreeNode;

/* B-tree node metadata */
typedef struct NodeMeta {
  uint16    t;              /* B-tree order (minimal children count)  */
  uint16    record_size;    /* size of a record                       */
  uint16    key_size;       /* size of the primary key                */
  uint16    key_position;   /* position of the primary key            */
} BtreeNodeMeta;

/* B-tree key comparison function */
typedef int (*CompareKeysPtr)(const void* key1,const void* key2,const size_t size);

/* B-tree node retrieval function */
typedef BtreeNode* (*BtreeLoadNodePtr)(const ulong position, const void* tree);

/* B-tree node write-out function */
typedef ulong (*BtreeWriteNodePtr)(const BtreeNode* node, const void* tree);

/* B-tree structure */
typedef struct {
  BtreeNodeMeta     meta;         /* node metadata                           */
  BtreeNode         *root;        /* pointer to root node (always in memory) */
  CompareKeysPtr    CompareKeys;  /* pointer to key comparison function      */
  BtreeLoadNodePtr  LoadNode;     /* node retrieval function                 */
  BtreeWriteNodePtr WriteNode;    /* node write-out function                 */
} Btree;

/* B-tree allocation and initialization */
Btree* BtreeCreateTree(
  const uint16 t,
  const uint16 record_size,
  const uint16 key_size,
  const uint16 key_position);

/* B-tree node allocation function */
BtreeNode *BtreeCreateNode(const Btree *tree);

/* B-tree search */
byte* BtreeSearch(const byte* key,const Btree* t);

/* B-tree insertion */
int BtreeInsert(const byte* key,const Btree* t);

/* B-tree insertion function return values */
#define BTREE_INSERT_COLLISION  -1
#define BTREE_INSERT_SUCCEEDED   1

#endif
