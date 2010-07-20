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
 * 22.03.2010
 *  Added utility macros for code readability improvement.
 *  The BtreeNode structure now contains a pointer to the B-tree structure
 *    (for parameter count reduction).
 * 24.03.2010
 *  Added forward declarations for the B-tree structures
 *    (no void* needed anymore).
 * 27.03.2010
 *  Updated all functions to use double pointers and return integers.
 * 05.03.2010
 *  Moved the CompareKeyPtr declaration to common.h.
 * 14.07.2010
 *  Changed record size and node size to be of type uint32 (long).
 * 15.07.2010
 *  Added root_position element to BtreeMeta structure.
 * 19.07.2010
 *  Updating the types to be uint32 or char* (before uint16 and byte*).
 *  Added dummy implementations for ReadNode, WriteNode and DeleteNode.
 * 20.07.2010
 *  Added new utility macro for calculating a B-tree's node size.
 */

#ifndef BTREE_H_INCLUDED
#define BTREE_H_INCLUDED

#include "common.h"

/* forward declarations of the B-tree structures */
typedef struct mdbBtreeMeta mdbBtreeMeta;
typedef struct mdbBtree     mdbBtree;
typedef struct mdbBtreeNode mdbBtreeNode;

/* B-tree node retrieval function */
typedef mdbBtreeNode* (*BtreeLoadNodePtr)
    (const uint32 position, mdbBtree* tree);

/* B-tree node write-out function */
typedef uint32 (*BtreeWriteNodePtr)(mdbBtreeNode* node);

/* B-tree node deletion function */
typedef void (*BtreeDeleteNodePtr)(mdbBtreeNode* node);

/* B-tree node meta-data */
struct mdbBtreeMeta
{
  uint32 record_size;     /* size of a record                       */
  uint32 key_size;        /* size of the primary key                */
  uint32 key_position;    /* position of the primary key            */
  uint32 root_position;   /* position of the root node in the file  */
  uint32 order;           /* B-tree order (minimal children count)  */
};

/* B-tree structure */
struct mdbBtree
{
  mdbBtreeMeta meta;              /* node meta-data                          */
  uint32 nodeSize;                /* size of a node                          */
  mdbBtreeNode* root;             /* pointer to root node (preloaded)        */
  CompareKeysPtr CompareKeys;     /* pointer to key comparison function      */
  BtreeLoadNodePtr ReadNode;      /* node retrieval implementation           */
  BtreeWriteNodePtr WriteNode;    /* node write-out implementation           */
  BtreeDeleteNodePtr DeleteNode;  /* node deletion implementation            */
};

/* B-tree node structure */
struct mdbBtreeNode
{
  mdbBtree* T;            /* pointer to the B-tree                  */
  char *data;             /* raw data of the node                   */
  uint32 *record_count;   /* pointer to count of records            */
  uint32 *is_leaf;        /* pointer to leaf/internal information   */
  uint32 *children;       /* pointer to child pointer array         */
  char *records;          /* pointer to records                     */
  uint32 position;        /* position of node (data file)           */
};

/* B-tree allocation and initialization */
int mdbBtreeCreateTree(mdbBtree** tree,
    const uint32 order,
    const uint32 record_size,
    const uint32 key_size,
    const uint32 key_position);

/* B-tree node allocation function */
int mdbBtreeAllocateNode(mdbBtreeNode** node, mdbBtree *tree);

/* B-tree search */
int mdbBtreeSearch(const char* key, char** record, mdbBtree* t);

/* B-tree insertion */
int mdbBtreeInsert(const char* record, mdbBtree* t);

/* B-tree deletion */
int mdbBtreeDelete(const char* key, mdbBtree* t);

/* Dummy implementations for ReadNode, WriteNode and DeleteNode (prototypes) */
mdbBtreeNode* mdbDummyReadNode(const uint32 position, mdbBtree* tree);
uint32 mdbDummyWriteNode(mdbBtreeNode* node);
void mdbDummyDeleteNode(mdbBtreeNode* node);

/* General return values */
#define MDB_BTREE_SUCCESS          1  /* B-tree operation succeeded         */
#define MDB_BTREE_NOROOT           0  /* B-tree contains no root pointer    */
#define MDB_BTREE_NOTFOUND        -1  /* non-existing key                   */

/* B-tree search function return values */
#define MDB_BTREE_SEARCH_FOUND      MDB_BTREE_SUCCESS
#define MDB_BTREE_SEARCH_NOTFOUND   MDB_BTREE_NOTFOUND

/* B-tree insertion function return values */
#define MDB_BTREE_INSERT_COLLISION  -2  /* tree already contains that key   */
#define MDB_BTREE_INSERT_NOROOT     MDB_BTREE_NOROOT
#define MDB_BTREE_INSERT_SUCCESS    MDB_BTREE_SUCCESS

/* B-tree deletion function return values */
#define MDB_BTREE_DELETE_NOTFOUND   MDB_BTREE_NOTFOUND
#define MDB_BTREE_DELETE_NOROOT     MDB_BTREE_NOROOT
#define MDB_BTREE_DELETE_SUCCESS    MDB_BTREE_SUCCESS
#define MDB_BTREE_DELETE_EMPTYROOT  2  /* B-tree root node has no records   */

/************************************************
 * Utility macros (for better code readability) *
 ************************************************/

/* Calculates and stores the node size of a B-tree */
#define BT_CALC_NODESIZE(tree) \
  (tree)->nodeSize = 2 * ((tree)->meta.order + 1) * sizeof(uint32) + \
  (2 * (tree)->meta.order - 1) * (tree)->meta.record_size

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
#define BT_KEYSIZE(node)    node->T->meta.key_size
#define BT_KEYPOS(node)     node->T->meta.key_position
#define BT_RECSIZE(node)    node->T->meta.record_size
#define BT_ORDER(node)      node->T->meta.order
#define BT_ROOTPOS(node)    node->T->meta.root_position

/* Tests whether the left key is greater, less or equal than the right one */
#define BT_KEYCMP(k1,op,k2,node) \
  (node->T->CompareKeys((k1),(k2),BT_KEYSIZE(node)) op 0)

/* Copies N records between two nodes */
#define BT_COPYRECORDS(dest,D,src,S,N) \
  memcpy(BT_RECORD(dest,(D)),BT_RECORD(src,(S)),(N)*BT_RECSIZE(src))

/* Moves N records in a node */
#define BT_MOVERECORDS(node,D,S,N) \
  memmove(BT_RECORD(node,(D)),BT_RECORD(node,(S)),(N)*BT_RECSIZE(node))

/* Copies N child pointers between two nodes */
#define BT_COPYCHILDREN(dest,D,src,S,N) \
  memcpy(&dest->children[(D)],&src->children[(S)],(N)*sizeof(uint32))

/* Moves N child pointers in a node */
#define BT_MOVECHILDREN(node,D,S,N) \
  memmove(&node->children[(D)],&node->children[(S)],(N)*sizeof(uint32))

/* Replaces the i-th key of a node with its predecessor */
#define BT_COPYPREDECESSOR(node,P,src) \
  memcpy(BT_RECORD(node,(P)),BT_RECORD(src,BT_COUNT(src)-1),BT_RECSIZE(node))

/* Replaces the i-th key of a node with its successor */
#define BT_COPYSUCCESSOR(node,P,src) \
  memcpy(BT_RECORD(node,(P)),src->records,BT_RECSIZE(node))

/**************************************************/

#endif
