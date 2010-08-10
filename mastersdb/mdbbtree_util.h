/*
 * mdbbtree_util.h
 *
 * B-tree utility macros to make code shorter and more readable.
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
 */

#ifndef MDBBTREE_UTIL_H_INCLUDED
#define MDBBTREE_UTIL_H_INCLUDED

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
#define BT_KEYPOS(node)     node->T->meta.key_position
#define BT_RECSIZE(node)    node->T->meta.record_size
#define BT_ORDER(node)      node->T->meta.order
#define BT_ROOTPOS(node)    node->T->meta.root_position

/* Tests whether the left key is greater, less or equal than the right one */
#define BT_KEYCMP(k1,op,k2,node,size) \
  (node->T->CompareKeys((k1),(k2),size) op 0)

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
