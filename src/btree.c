
/*
 * btree.c
 *
 * B-tree function implementations (see btree.h)
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
 *  Initial version of file.
 * 02.03.2010
 *  Implemented B-tree search.
 * 07.03.2010
 *  Implemented B-tree insertion.
 */

#include "btree.h"
#include <malloc.h>

/*
 * B-tree allocation and initialization
 */
Btree* BtreeCreateTree(
  const uint16 t,
  const uint16 record_size,
  const uint16 key_size,
  const uint16 key_position) {

  /* allocates a B-tree structure */
  Btree *tree = (Btree*)malloc(sizeof(Btree));

  /* initializes the B-tree structure */
  tree->meta.t = t;
  tree->meta.record_size = record_size;
  tree->meta.key_size = key_size;
  tree->meta.key_position = key_position;

  return tree;
}

/*
 * B-tree node allocation function
 *
 * The node data can be described as following:
 *
 * PART     | record count + is leaf | children | records                 |
 * SIZE     | 4                      | 4 * T*2  | RECORD_SIZE * (T*2 - 1) |
 * POSITION | 0                      | 4        | 4 * (T*2 + 1)           |
 *
 * The node size (in bytes) can be determined as following:
 *  NODE_SIZE = (2 * T) * (RECORD_SIZE + 4) - RECORD_SIZE + 4
 *
*/
BtreeNode *BtreeCreateNode(const Btree *tree) {

  BtreeNode *node= (BtreeNode*)malloc(sizeof(BtreeNode));
  ulong node_size = 2 * tree->meta.t * (tree->meta.record_size + 4)
                    - tree->meta.record_size + 4;

  /* Allocates the memory */
  node->data = (byte*)malloc(node_size);

  /* Initializes the data pointers */
  node->record_count = (uint16*)node->data;
  node->is_leaf = node->record_count + 1;
  node->children = (ulong*)(node->is_leaf + 1);
  node->records = (byte*)(node->children + tree->meta.t * 2);

  return node;
}

/*
 * Recursive B-tree search (internal, not visible to the programmer)
*/
byte* BtreeSearchRecursive(
  const byte* key, const BtreeNode* node, const Btree* t)
{
  BtreeNode* next = NULL;
  byte* res = NULL;
  uint16 i = 0;

  while (i < *node->record_count
    && t->CompareKeys(key,node->records + i*t->meta.record_size
      + t->meta.key_position,t->meta.key_size) > 0)
  {
    i++;
  }

  /* key/record found? */
  if (i < *node->record_count) {
    if (t->CompareKeys(key,node->records + i*t->meta.record_size
      + t->meta.key_position,t->meta.key_size) == 0)
    {
      res = (byte*)malloc(t->meta.record_size);
      memcpy(res,node->records + i*t->meta.record_size
        + t->meta.key_position,t->meta.record_size);
      return res;
    }
  }

  /* if node is a leaf, search for the key and return NULL if not found */
  if (*node->is_leaf == 1) {
    res = NULL;
  }
  /* if node is an internal node, search for the key or recurse to subtree */
  else {
    next = t->LoadNode(node->children[i],t);
    res = BtreeSearchRecursive(key,next,t);
    free(next->data);
    free(next);
  }

  return res;
}

/*
 * B-tree search function
*/
byte* BtreeSearch(const byte* key,const Btree* t) {
  if (t->root != NULL) {
    return BtreeSearchRecursive(key,t->root,t);
  } else {
    return NULL;
  }
}


/*
 * Recursive B-tree insertion (internal, not visible to the programmer)
*/
int BtreeInsertRecursive(
  const byte* record, BtreeNode* node, const Btree* t)
{
  BtreeNode* next = NULL;
  uint16 i = 0;
  int res = 0;

  while (i < *node->record_count
    && t->CompareKeys(record + t->meta.key_position,
        node->records + i*t->meta.record_size + t->meta.key_position,
        t->meta.key_size) > 0)
  {
    i++;
  }

  /* if the current node is a leaf, insert the record into it */
  if (*node->is_leaf == 1) {

    /* key/record already exists? */
    if (i < *node->record_count) {
      if (t->CompareKeys(record + t->meta.key_position, node->records
            + i*t->meta.record_size + t->meta.key_position,t->meta.key_size)
          == 0)
      {
        return BTREE_INSERT_COLLISION;
      }
    }

    /* if needed, create space for the new record */
    if (i < *node->record_count) {
      memmove(node->records + (i+1)*t->meta.record_size,
        node->records + i*t->meta.record_size,
        (*node->record_count - i) * t->meta.record_size);
    }
    memcpy(node->records + i*t->meta.record_size,record,t->meta.record_size);
    *node->record_count = (*node->record_count + 1);
    return BTREE_INSERT_SUCCEEDED;
  }

  /* if node is an internal node, check if the child node is full
   * recurse to the subtree */
  else {
    return 0;
    /*
    next = t->LoadNode(node->children[i],t);
    res = BtreeSearchRecursive(key,next,t);
    free(next->data);
    free(next);
    */
  }

  return res;
}

/*
 * B-tree insertion function
*/
int BtreeInsert(const byte* key,const Btree* t) {
  if (t->root != NULL) {
    return BtreeInsertRecursive(key,t->root,t);
  } else {
    return 0;
  }
}


