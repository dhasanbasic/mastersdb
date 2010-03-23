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
 *  Started implementation of B-tree insertion.
 * 14.03.2010
 *  Implemented internal BtreeSplitNode function.
 * 17.03.2010
 *  Finished B-tree insertion.
 *  Started B-tree deletion.
 * 19.03.2010
 *  Implemented BtreeMerge utility function.
 * 20.03.2010
 *  Finished B-tree deletion.
 *  Fixed a bug in BtreeInsert (root split)
 * 23.03.2010
 *  Updated code to reflect changes in the header file (utility macros etc.).
 */

#include "btree.h"
#include <malloc.h>

/* TODO: Optimization: binary search during insert/delete                 */
/* TODO: Optimization: write iterative versions of insert/delete          */
/* TODO: Optimization: No allocation in ReadNode/WriteNode call           */

/*
 * B-tree allocation and initialization
 */
Btree* BtreeCreateTree(const uint16 t, const uint16 record_size,
    const uint16 key_size, const uint16 key_position)
{

  /* allocates a B-tree structure */
  Btree *tree = (Btree*) malloc(sizeof(Btree));

  /* initializes the B-tree structure */
  tree->meta.t = t;
  tree->meta.record_size = record_size;
  tree->meta.key_size = key_size;
  tree->meta.key_position = key_position;
  tree->nodeSize = 2 * tree->meta.t * (tree->meta.record_size + 4)
      - tree->meta.record_size + 4;

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
BtreeNode *BtreeAllocateNode(Btree *tree)
{

  BtreeNode *node = (BtreeNode*) malloc(sizeof(BtreeNode));

  /* Allocates the memory */
  node->data = (byte*) malloc(tree->nodeSize);
  memset(node->data, 0, tree->nodeSize);

  /* Initializes the data pointers */
  node->record_count = (uint16*) node->data;
  node->is_leaf = node->record_count + 1;
  node->children = (ulong*) (node->is_leaf + 1);
  node->records = (byte*) (node->children + tree->meta.t * 2);
  node->T = tree;
  node->position = 0L;

  return node;
}

/*
 * Recursive B-tree search (internal, not visible to the programmer)
 */
byte* BtreeSearchRecursive(const byte* key, BtreeNode* node)
{
  BtreeNode* next = NULL;
  byte* res = NULL;
  uint16 i = 0;

  while (i < BT_COUNT(node) && BT_KEYCMP(key,>,BT_KEY(node,i),node))
  {
    i++;
  }

  /* key/record found? */
  if (i < BT_COUNT(node))
  {
    if (BT_KEYCMP(key,==,BT_KEY(node,i),node))
    {
      res = (byte*) malloc(BT_RECSIZE(node));

      memcpy(res, BT_RECORD(node,i), BT_RECSIZE(node));
      return res;
    }
  }

  /* if node is a leaf, search for the key and return NULL if not found */
  if (BT_LEAF(node))
  {
    res = NULL;
  }
  /* if node is an internal node, search for the key or recurse to subtree */
  else
  {
    next = node->T->ReadNode(node->children[i], node->T);
    res = BtreeSearchRecursive(key, next);
    free(next->data);
    free(next);
  }

  return res;
}

/*
 * B-tree search function
 */
byte* BtreeSearch(const byte* key, Btree* t)
{
  if (t->root != NULL)
  {
    return BtreeSearchRecursive(key, t->root);
  }
  else
  {
    return NULL;
  }
}

/*
 * Internal function for splitting a full node
 */
void BtreeSplitNode(BtreeNode* left, BtreeNode* parent, const uint16 position)
{
  BtreeNode* right = BtreeAllocateNode(left->T);

  /* copy the second half parts of the records and child pointers from the
   * left child node to the new right child node
   */
  memcpy(right->records, left->records + left->T->meta.t * left->T->meta.record_size,
      (left->T->meta.t - 1) * left->T->meta.record_size);

  memcpy(right->children, &left->children[left->T->meta.t], left->T->meta.t * sizeof(ulong));

  *right->is_leaf = *left->is_leaf;
  *right->record_count = left->T->meta.t - 1;
  right->position = 0L;

  /* update the left child node */
  *left->record_count = *right->record_count;

  /* move the median record to the parent node, make space for it if needed */
  if (position < (*parent->record_count - 1))
  {
    memmove(parent->records + (position + 1) * parent->T->meta.record_size,
        parent->records + position * parent->T->meta.record_size,
        (*parent->record_count - position) * parent->T->meta.record_size);

    memmove(&parent->children[position + 2], &parent->children[position + 1],
        (*parent->record_count - position) * sizeof(ulong));
  }
  memcpy(parent->records + position * parent->T->meta.record_size, left->records
      + (parent->T->meta.t - 1) * parent->T->meta.record_size, parent->T->meta.record_size);

  *parent->record_count = (*parent->record_count + 1);

  /* save all changes */
  parent->children[position] = parent->T->WriteNode(left, parent->T);
  parent->children[position + 1] = parent->T->WriteNode(right, parent->T);
  parent->position = parent->T->WriteNode(parent, parent->T);
  free(right->data);
  free(right);
}

/*
 * Recursive B-tree insertion (internal, not visible to the programmer)
 */
int BtreeInsertRecursive(const byte* record, BtreeNode* node)
{
  Btree* t = node->T;
  BtreeNode* next = NULL;
  uint16 i = 0;
  int result = 0;

  while (i < *node->record_count && t->CompareKeys(record
      + t->meta.key_position, node->records + i * t->meta.record_size
      + t->meta.key_position, t->meta.key_size) > 0)
  {
    i++;
  }

  /* key already exists? */
  if (i < *node->record_count)
  {
    if (t->CompareKeys(record + t->meta.key_position, node->records + i
        * t->meta.record_size + t->meta.key_position, t->meta.key_size) == 0)
    {
      return BTREE_INSERT_COLLISION;
    }
  }

  /* if the current node is a leaf, insert the record into it */
  if (*node->is_leaf > 0)
  {
    /* if needed, create space for the new record */
    if (i < *node->record_count)
    {
      memmove(node->records + (i + 1) * t->meta.record_size, node->records + i
          * t->meta.record_size, (*node->record_count - i)
          * t->meta.record_size);
    }
    memcpy(node->records + i * t->meta.record_size, record, t->meta.record_size);
    *node->record_count = (*node->record_count + 1);
    node->position = t->WriteNode(node, t);
    return BTREE_INSERT_SUCCEEDED;
  }

  /* if node is an internal node recurse to the subtree */
  else
  {
    next = t->ReadNode(node->children[i], t);

    /* child node is full, split it */
    if (*next->record_count == t->meta.t * 2 - 1)
    {
      BtreeSplitNode(next, node, i);

      /* the current key changed and the current subtree maybe smaller than
       * then the record's key, so determine the direction of the recursion
       * (left or right) */
      if (t->CompareKeys(record + t->meta.key_position, node->records + i
          * t->meta.record_size + t->meta.key_position, t->meta.key_size) > 0)
      {
        free(next->data);
        free(next);
        next = t->ReadNode(node->children[i + 1], t);
      }
    }

    result = BtreeInsertRecursive(record, next);
    free(next->data);
    free(next);
    return result;
  }
}

/*
 * B-tree insertion function
 */
int BtreeInsert(const byte* record, Btree* t)
{
  BtreeNode* root = t->root;
  BtreeNode* newRoot = NULL;

  if (root != NULL)
  {
    /* root node is full, split it */
    if (*root->record_count == t->meta.t * 2 - 1)
    {
      newRoot = BtreeAllocateNode(t);

      /* the new root will be an internal node, while the old root
       * will become a leaf node and get a right sibling
       */
      newRoot->position = 0L;
      *newRoot->is_leaf = 0;

      BtreeSplitNode(root, newRoot, 0);

      /* free up used resources */
      free(root->data);
      free(root);
      t->root = newRoot;
    }
    return BtreeInsertRecursive(record, t->root);
  }
  else
  {
    return BTREE_INSERT_NOROOT;
  }
}

/*
 * Internal function for merging two child nodes with median from parent
 */
void BtreeMergeNodes(BtreeNode* left, BtreeNode* right, BtreeNode* parent,
    const uint16 median)
{
  Btree* t = parent->T;
  /* -----------------------------------------------------------------*/
  /* update the left child node */
  /* -----------------------------------------------------------------*/
  /* copy the median key to the left child node */
  memcpy(left->records + (t->meta.t - 1) * t->meta.record_size, parent->records
      + median * t->meta.record_size, t->meta.record_size);

  /* copy all records from the right child to the left child */
  memcpy(left->records + t->meta.t * t->meta.record_size, right->records,
      *right->record_count * t->meta.record_size);

  /* if needed, copy all child pointers from the right to the left child */
  if (*right->is_leaf < 1)
  {
    memcpy(&left->children[t->meta.t], right->children, t->meta.t
        * sizeof(ulong));
  }

  *left->record_count = t->meta.t * 2 - 1;
  /* -----------------------------------------------------------------*/

  /* -----------------------------------------------------------------*/
  /* update the parent node */
  /* -----------------------------------------------------------------*/

  /* if there are records (and child pointers) to be shifted */
  if ((*parent->record_count - median - 1) > 0)
  {
    /* shift the records after the median to left by one */
    memmove(parent->records + median * t->meta.record_size, parent->records
        + (median + 1) * t->meta.record_size, (*parent->record_count - median
        - 1) * t->meta.record_size);

    /* shift the child pointers after right child to left by one */
    memmove(&parent->children[median + 1], &parent->children[median + 2],
        (*parent->record_count - median - 1) * sizeof(ulong));
  }

  *parent->record_count = (*parent->record_count - 1);
  /* -----------------------------------------------------------------*/

  /* save the changes */
  parent->children[median] = t->WriteNode(left, t);
  parent->position = t->WriteNode(parent, t);
  t->DeleteNode(right, t);
  free(right->data);
  free(right);
}

/*
 * Internal recursive B-tree deletion function
 */
int BtreeDeleteRecursive(const byte* key, BtreeNode* node)
{
  Btree* t = node->T;
  BtreeNode* left = NULL;
  BtreeNode* right = NULL;
  BtreeNode* next = NULL;
  uint16 i = 0;
  int result = 0;

  while (i < *node->record_count && t->CompareKeys(key + t->meta.key_position,
      node->records + i * t->meta.record_size + t->meta.key_position,
      t->meta.key_size) > 0)
  {
    i++;
  }

  /* key found? */
  if (i < *node->record_count && t->CompareKeys(key + t->meta.key_position,
      node->records + i * t->meta.record_size + t->meta.key_position,
      t->meta.key_size) == 0)
  {
    /* if current node is a leaf, simply remove the record */
    if (*node->is_leaf > 0)
    {
      /* if there are records to be shifted */
      if ((*node->record_count - i - 1) > 0)
      {
        /* shift the records after the median to left by one */
        memmove(node->records + i * t->meta.record_size, node->records
            + (i + 1) * t->meta.record_size, (*node->record_count - i - 1)
            * t->meta.record_size);
      }
      *node->record_count = (*node->record_count - 1);
      node->position = t->WriteNode(node, t);
    }
    /* otherwise remove the record without destroying the tree's balance */
    else
    {
      /*
       * If possible (left child node before the key to be deleted
       * contains at least T keys), replace the key to be deleted by
       * its PREDECESSOR
       */
      left = t->ReadNode(node->children[i], t);
      if (*left->record_count >= t->meta.t)
      {
        memcpy(node->records + i * t->meta.record_size, left->records
            + (*left->record_count - 1) * t->meta.record_size,
            t->meta.record_size);
        node->position = t->WriteNode(node, t);

        result = BtreeDeleteRecursive(left->records + (*left->record_count - 1)
            * t->meta.record_size + t->meta.key_position, left);

        free(left->data);
        free(left);

        return result;
      }

      /*
       * Otherwise, if possible (right child node before the key to be
       * deleted contains at least T keys), replace the key to be deleted
       * by its SUCCESSOR
       */
      right = t->ReadNode(node->children[i + 1], t);
      if (*right->record_count >= t->meta.t)
      {
        memcpy(node->records + i * t->meta.record_size, right->records,
            t->meta.record_size);
        node->position = t->WriteNode(node, t);

        result = BtreeDeleteRecursive(right->records + t->meta.key_position,
            right);

        free(right->data);
        free(right);
        free(left->data);
        free(left);

        return result;
      }

      /* If both the left and right child nodes contains T - 1 keys,
       * merge them with the key to be deleted as the median key
       */
      BtreeMergeNodes(left, right, node, i);

      result = BtreeDeleteRecursive(key, left);
      free(left->data);
      free(left);
      return result;
    }
    return BTREE_DELETE_SUCCEEDED;
  }
  else
  {
    /* if current node is leaf, the record does not exist */
    if (*node->is_leaf > 0)
    {
      return BTREE_DELETE_NOTFOUND;
    }
    /* otherwise, re-balance the tree if necessary and recurse to subtree */
    else
    {
      next = t->ReadNode(node->children[i], t);

      /* if new subtree has only T - 1 keys, re-balance the tree */
      if (*next->record_count < t->meta.t)
      {
        left = NULL;
        /* if a left sibling of the subtree root node
         * exists and has a key to spare...
         */
        if (i > 0)
        {
          left = t->ReadNode(node->children[i - 1], t);

          if (*left->record_count >= t->meta.t)
          {
            /* ---------------------------------------------------------- *
             * perform a rotate operation from the left sibling,          *
             * through the parent node, into the subtree root node        *
             * ---------------------------------------------------------- */

            /* create space for the spared key/record */
            memmove(next->records + t->meta.record_size, next->records,
                (*next->record_count) * t->meta.record_size);

            /* insert the current key/record into the subtree root node */
            memcpy(next->records, node->records + i * t->meta.record_size,
                t->meta.record_size);

            /* replace the current key/record by the last key/record
             * from the left sibling
             */
            memcpy(node->records + i * t->meta.record_size, left->records
                + (*left->record_count - 1) * t->meta.record_size,
                t->meta.record_size);

            /* if the children are internal nodes, move the left
             * sibling's last child pointer to the subtree root node
             */
            if (*next->is_leaf < 1)
            {
              memmove(&next->children[1], &next->children[0],
                  (*next->record_count + 1) * sizeof(ulong));
              next->children[0] = left->children[*left->record_count];
            }

            /* update all nodes */
            *next->record_count = (*next->record_count + 1);
            *left->record_count = (*left->record_count - 1);

            node->children[i - 1] = t->WriteNode(left, t);
            node->children[i] = t->WriteNode(next, t);
            node->position = t->WriteNode(node, t);

            free(left->data);
            free(left);
            /* ---------------------------------------------------------- */
            goto BTREE_DELETE_RECURSE;
          }
        }

        /* otherwise, if the right sibling of the
         * subtree root has a key to spare...
         */
        right = t->ReadNode(node->children[i + 1], t);

        if (*right->record_count >= t->meta.t)
        {
          /* ---------------------------------------------------------- *
           * perform a reverse rotate operation from the right          *
           * sibling, through the parent node, into the subtree         *
           * root node                                                  *
           * ---------------------------------------------------------- */

          /* insert the current key/record into the subtree root node */
          memcpy(next->records + (*next->record_count) * t->meta.record_size,
              node->records + i * t->meta.record_size, t->meta.record_size);

          /* replace the current key/record by the first key/record
           * from the right sibling
           */
          memcpy(node->records + i * t->meta.record_size, right->records,
              t->meta.record_size);

          /* if the children are internal nodes, move the right
           * sibling's first child pointer to the subtree root node
           */
          if (*next->is_leaf < 1)
          {
            next->children[*next->record_count + 1] = right->children[0];
            memmove(&right->children[0], &right->children[1],
                (*right->record_count) * sizeof(ulong));
          }

          /* shift all records of the right sibling to the left */
          memmove(right->records, right->records + t->meta.record_size,
              (*right->record_count - 1) * t->meta.record_size);

          /* update all nodes */
          *next->record_count = (*next->record_count + 1);
          *right->record_count = (*right->record_count - 1);

          node->children[i + 1] = t->WriteNode(right, t);
          node->children[i] = t->WriteNode(right, t);
          node->position = t->WriteNode(node, t);

          free(right->data);
          free(right);
          /* ---------------------------------------------------------- */
          goto BTREE_DELETE_RECURSE;
        }

        /* if both the left and right siblings of the subtree root
         * node contain T - 1 keys, perform a merge
         */
        if (left != NULL)
        {

          BtreeMergeNodes(left, next, node, i);

          /* SPECIAL CASE: root of B-tree became empty */
          if (*node->record_count > 0)
          {
            if (t->CompareKeys(key, node->records + i * t->meta.record_size
                + t->meta.key_position, t->meta.key_size) > 0)
            {
              free(left->data);
              free(left);
              next = right;
            }
            else
            {
              free(right->data);
              free(right);
              next = left;
            }
          }
          else
          {
            t->DeleteNode(node, t);
            t->root = left;
          }
        }
        else
        {
          BtreeMergeNodes(next, right, node, i);

          /* SPECIAL CASE: root of B-tree became empty */
          if (*node->record_count > 0)
          {
            if (t->CompareKeys(key, node->records + i * t->meta.record_size
                + t->meta.key_position, t->meta.key_size) > 0)
            {
              free(next->data);
              free(next);
              next = t->ReadNode(node->children[i + 1], t);
            }
          }
          else
          {
            t->DeleteNode(node, t);
            t->root = next;
          }
        }
      }

      BTREE_DELETE_RECURSE:

      result = BtreeDeleteRecursive(key, next);
      if (next != t->root)
      {
        free(next->data);
        free(next);
      }
      return result;
    }
  }
}

/*
 * B-tree deletion function
 */
int BtreeDelete(const byte* key, Btree* t)
{
  BtreeNode* root = t->root;
  if (root != NULL)
  {
    /* root node is empty */
    if (*root->record_count == 0)
    {
      return BTREE_DELETE_EMPTYROOT;
    }
    return BtreeDeleteRecursive(key, root);
  }
  else
  {
    return BTREE_DELETE_NOROOT;
  }
}
