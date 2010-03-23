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
 *  Started implementation of B-tree insertion.
 * 14.03.2010
 *  Implemented internal BtreeSplitNode function.
 * 17.03.2010
 *  Finished B-tree insertion.
 *  Started B-tree deletion.
 * 20.03.2010
 *  Finished B-tree deletion.
 *  Fixed a bug in BtreeInsert (root split)
 * 22.03.2010
 *  Added utility macros for code readability improvement.
 *  The BtreeNode structure now contains a pointer to the B-tree structure
 *    (for parameter count reduction).
 */

#ifndef BTREE_H_INCLUDED
#define BTREE_H_INCLUDED

#include "types.h"
#include <string.h>

/* B-tree key comparison function */
typedef int (*CompareKeysPtr)(const void* key1, const void* key2,
    const size_t size);

/* B-tree node retrieval function */
typedef void* (*BtreeLoadNodePtr)(const ulong position, const void* tree);

/* B-tree node write-out function */
typedef ulong (*BtreeWriteNodePtr)(void* node, const void* tree);

/* B-tree node deletion function */
typedef void (*BtreeDeleteNodePtr)(void* node, const void* tree);

/* B-tree node meta-data */
typedef struct NodeMeta
{
  uint16 t;               /* B-tree order (minimal children count)  */
  uint16 record_size;     /* size of a record                       */
  uint16 key_size;        /* size of the primary key                */
  uint16 key_position;    /* position of the primary key            */
} BtreeNodeMeta;

/* B-tree structure */
typedef struct
{
  BtreeNodeMeta meta;             /* node metadata                           */
  void *root;                     /* pointer to root node (preloaded)        */
  uint16 nodeSize;                /* size of a node                          */
  CompareKeysPtr CompareKeys;     /* pointer to key comparison function      */
  BtreeLoadNodePtr ReadNode;      /* node retrieval implementation           */
  BtreeWriteNodePtr WriteNode;    /* node write-out implementation           */
  BtreeDeleteNodePtr DeleteNode;  /* node deletion implementation            */
} Btree;

/* B-tree node structure */
typedef struct
{
  Btree* T;               /* pointer to the B-tree                  */
  byte *data;             /* raw data of the node                   */
  uint16 *record_count;   /* pointer to count of records            */
  uint16 *is_leaf;        /* pointer to leaf/internal information   */
  ulong *children;        /* pointer to child pointer array         */
  byte *records;          /* pointer to records                     */
  ulong position;         /* position of node (data file)           */
} BtreeNode;

/* B-tree allocation and initialization */
Btree* BtreeCreateTree(const uint16 t, const uint16 record_size,
    const uint16 key_size, const uint16 key_position);

/* B-tree node allocation function */
BtreeNode *BtreeAllocateNode(Btree *tree);

/* B-tree search */
byte* BtreeSearch(const byte* key, Btree* t);

/* B-tree insertion */
int BtreeInsert(const byte* record, Btree* t);

/* B-tree deletion */
int BtreeDelete(const byte* key, Btree* t);

/* B-tree insertion function return values */
#define BTREE_INSERT_COLLISION    -1  /* B-tree already contains that key   */
#define BTREE_INSERT_NOROOT        0  /* B-tree has no root defined (null)  */
#define BTREE_INSERT_SUCCEEDED     1  /* B-tree insertion succeeded         */

/* B-tree deletion function return values */
#define BTREE_DELETE_NOTFOUND     -1  /* B-tree does not contain that key   */
#define BTREE_DELETE_NOROOT        0  /* B-tree has no root defined (null)  */
#define BTREE_DELETE_SUCCEEDED     1  /* B-tree deletion succeeded          */
#define BTREE_DELETE_EMPTYROOT     2  /* B-tree root node has no records    */

/************************************************
 * Utility macros (for better code readability) *
 ************************************************/

/* Returns a pointer to the i-th record of a node */
#define BT_RECORD(node,i)   (node->records + (i) * BT_RECSIZE(node))

/* Returns a pointer to the i-th key in a node */
#define BT_KEY(node,i)      (BT_RECORD(node,i) + BT_KEYPOS(node))

/* Tests whether the given node is a leaf or internal node */
#define BT_LEAF(node)       (*node->is_leaf > 0)
#define BT_INTERNAL(node)   (*node->is_leaf < 1)

/* Returns the number of records in a node */
#define BT_COUNT(node)      (*node->record_count)

/* B-tree node meta-data shortcuts */
#define BT_KEYSIZE(node)    (node->T->meta.key_size)
#define BT_KEYPOS(node)     (node->T->meta.key_position)
#define BT_RECSIZE(node)    (node->T->meta.record_size)
#define BT_ORDER(node)      (node->T->meta.t)

/* Returns pointers to the first or last records of a node */
#define BT_LAST(node)       (node->records)
#define BT_FIRST(node)      (node->records + (BT_COUNT(node) - 1) * \
                             BT_RECSIZE(node))

/* Tests whether the left key is greater, less or equal than the right one */
#define BT_KEYCMP(k1,op,k2,node) \
  (node->T->CompareKeys((k1),(k2),BT_KEYSIZE(node)) op 0)

/* Copies N records between two nodes */
#define BT_COPYRECORDS(D,dest,S,src,N) \
  memcpy(BT_RECORD(dest,(D)),BT_RECORD(src,(S)),(N)*BT_RECSIZE(src))

/* Moves N records in a node */
#define BT_MOVERECORDS(D,S,node,N) \
  memmove(BT_RECORD(node,(D)),BT_RECORD(node,(S)),(N)*BT_RECSIZE(node))

/* Copies N child pointers between two nodes */
#define BT_COPYCHILDREN(D,dest,S,src,N) \
  memcpy(&dest->children[(D)],&src->children[(S)],(N)*sizeof(ulong))

/* Moves N child pointers in a node */
#define BT_MOVECHILDREN(D,S,node,N) \
  memmove(&node->children[(D)],&node->children[(S)],(N)*sizeof(ulong))


/**************************************************/

#endif
