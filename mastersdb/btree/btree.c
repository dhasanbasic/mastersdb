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
 * 26.03.2010
 *  Fixed a bug in insertion (no collision detected after split operation).
 *  Fixed several bugs in deletion.
 * 27.03.2010
 *  Updated code to reflect changes (double pointers, integer return values).
 */

#include "btree.h"

/* TODO: Optimization: binary search during insert/delete                 */
/* TODO: Optimization: write iterative versions of insert/delete          */
/* TODO: Optimization: No allocation in ReadNode/WriteNode call           */

/*
 * B-tree allocation and initialization
 */
int BtreeCreateTree(Btree** tree, const uint16 t, const uint16 record_size,
    const uint16 key_size, const uint16 key_position)
{
  /* allocates a B-tree structure */
  *tree = (Btree*) malloc(sizeof(Btree));

  /* initializes the B-tree structure */
  (*tree)->meta.t = t;
  (*tree)->meta.record_size = record_size;
  (*tree)->meta.key_size = key_size;
  (*tree)->meta.key_position = key_position;
  (*tree)->nodeSize = ((*tree)->meta.t << 1) * ((*tree)->meta.record_size + 4)
      - (*tree)->meta.record_size + 4;

  return BTREE_SUCCESS;
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
int BtreeAllocateNode(BtreeNode** node, Btree *tree)
{
  *node = (BtreeNode*) malloc(sizeof(BtreeNode));

  /* Allocates the memory */
  (*node)->data = (byte*) malloc(tree->nodeSize);
  memset((*node)->data, 0, tree->nodeSize);

  /* Initializes the data pointers */
  (*node)->record_count = (uint16*) (*node)->data;
  (*node)->is_leaf = (*node)->record_count + 1;
  (*node)->children = (ulong*) ((*node)->is_leaf + 1);
  (*node)->records = (byte*) ((*node)->children + (tree->meta.t << 1));
  (*node)->T = tree;
  (*node)->position = 0L;

  return BTREE_SUCCESS;
}

/*
 * Recursive B-tree search (internal, not visible to the programmer)
 */
int BtreeSearchRecursive(const byte* key, byte** record, BtreeNode* node)
{
  BtreeNode* next = NULL;
  int result = 0;
  uint16 i = 0;

  while (i < BT_COUNT(node) && BT_KEYCMP(key, > ,BT_KEY(node,i),node))
  {
    i++;
  }

  /* key/record found? */
  if (i < BT_COUNT(node))
  {
    if (BT_KEYCMP(key, == ,BT_KEY(node,i),node))
    {
      *record = (byte*) malloc(BT_RECSIZE(node));
      memcpy(*record, BT_RECORD(node,i), BT_RECSIZE(node));
      return BTREE_SEARCH_FOUND;
    }
  }

  /* if node is a leaf, search for the key and return NULL if not found */
  if (BT_LEAF(node))
  {
    *record = NULL;
    result = BTREE_SEARCH_NOTFOUND;
  }
  /* if node is an internal node, search for the key or recurse to subtree */
  else
  {
    next = node->T->ReadNode(node->children[i], node->T);
    result = BtreeSearchRecursive(key, record, next);
    free(next->data);
    free(next);
  }

  return result;
}

/*
 * B-tree search function
 */
int BtreeSearch(const byte* key, byte** record, Btree* t)
{
  if (t->root != NULL)
  {
    return BtreeSearchRecursive(key, record, t->root);
  }
  else
  {
    return BTREE_INSERT_NOROOT;
  }
}

/*
 * Internal function for splitting a full node
 */
void BtreeSplitNode(BtreeNode* left, BtreeNode* parent, const uint16 position)
{
  BtreeNode* right = NULL;
  BtreeAllocateNode(&right, left->T);

  /* copy the second half parts of the records and child pointers from the
   * left child node to the new right child node
   */

  BT_COPYRECORDS(right, 0, left, BT_ORDER(left), BT_ORDER(left) - 1);
  BT_COPYCHILDREN(right, 0, left, BT_ORDER(left), BT_ORDER(left));

  *right->is_leaf = *left->is_leaf;
  right->position = 0L;
  BT_COUNT(right) = BT_ORDER(left) - 1;

  /* update the left child node */
  BT_COUNT(left) = BT_COUNT(right);

  /* move the median record to the parent node, make space for it if needed */
  if (position < BT_COUNT(parent))
  {
    BT_MOVERECORDS(parent, position + 1, position, BT_COUNT(parent) - position);
    BT_MOVECHILDREN(parent, position + 2, position + 1,
        BT_COUNT(parent) - position);
  }

  memcpy(BT_RECORD(parent, position), BT_RECORD(left,BT_ORDER(left) - 1),
      BT_RECSIZE(parent));

  BT_COUNT(parent) = BT_COUNT(parent) + 1;

  /* save all changes */
  parent->children[position] = parent->T->WriteNode(left);
  parent->children[position + 1] = parent->T->WriteNode(right);
  parent->position = parent->T->WriteNode(parent);
  free(right->data);
  free(right);
}

/*
 * Recursive B-tree insertion (internal, not visible to the programmer)
 */
int BtreeInsertRecursive(const byte* record, BtreeNode* node)
{
  BtreeNode* next = NULL;
  uint16 i = 0;
  int result = 0;

  while (i < BT_COUNT(node) &&
      BT_KEYCMP(record + BT_KEYPOS(node), > ,BT_KEY(node,i), node))
  {
    i++;
  }

  /* key already exists? */
  if (i < BT_COUNT(node))
  {
    if (BT_KEYCMP(record + BT_KEYPOS(node), == ,BT_KEY(node,i), node))
    {
      return BTREE_INSERT_COLLISION;
    }
  }

  /* if the current node is a leaf, insert the record into it */
  if (BT_LEAF(node))
  {
    /* if needed, create space for the new record */
    if (i < BT_COUNT(node))
    {
      BT_MOVERECORDS(node, i+1, i, BT_COUNT(node) - i);
    }
    memcpy(BT_RECORD(node, i), record, BT_RECSIZE(node));
    BT_COUNT(node) = BT_COUNT(node) + 1;
    node->position = node->T->WriteNode(node);
    return BTREE_INSERT_SUCCESS;
  }

  /* if node is an internal node recurse to the subtree */
  else
  {
    next = node->T->ReadNode(node->children[i], node->T);

    /* child node is full, split it */
    if (BT_COUNT(next) == (BT_ORDER(node) << 1) - 1)
    {
      BtreeSplitNode(next, node, i);

      /* the current key changed and the current subtree maybe smaller than
       * then the record's key, so determine the direction of the recursion
       * (left or right) */
      if (BT_KEYCMP(record + BT_KEYPOS(node), > ,BT_KEY(node,i), node))
      {
        free(next->data);
        free(next);
        next = node->T->ReadNode(node->children[i + 1], node->T);
      }
      else if (BT_KEYCMP(record + BT_KEYPOS(node), == ,BT_KEY(node,i), node))
      {
        free(next->data);
        free(next);
        return BTREE_INSERT_COLLISION;
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
  BtreeNode* newRoot = NULL;

  if (t->root != NULL)
  {
    /* t->root node is full, split it */
    if (BT_COUNT(t->root) == (BT_ORDER(t->root) << 1) - 1)
    {
      BtreeAllocateNode(&newRoot, t);

      /* the new t->root will be an internal node, while the old t->root
       * will become a leaf node and get a right sibling
       */
      newRoot->position = 0L;
      *newRoot->is_leaf = 0;

      BtreeSplitNode(t->root, newRoot, 0);

      /* free up used resources */
      free(t->root->data);
      free(t->root);
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
  /* -----------------------------------------------------------------*/
  /* update the left child node */
  /* -----------------------------------------------------------------*/
  /* copy the median key to the left child node */
  BT_COPYRECORDS(left, BT_ORDER(left) - 1, parent, median, 1);

  /* copy all records from the right child to the left child */
  BT_COPYRECORDS(left, BT_ORDER(left), right, 0, BT_COUNT(right));

  /* if needed, copy all child pointers from the right to the left child */
  if (BT_INTERNAL(right))
  {
    BT_COPYCHILDREN(left, BT_ORDER(left), right, 0, BT_ORDER(left));
  }

  BT_COUNT(left) = (BT_ORDER(left) << 1) - 1;
  /* -----------------------------------------------------------------*/

  /* -----------------------------------------------------------------*/
  /* update the parent node */
  /* -----------------------------------------------------------------*/

  /* if there are records (and child pointers) to be shifted */
  if ((BT_COUNT(parent) - median - 1) > 0)
  {
    /* shift the records after the median to left by one */
    BT_MOVERECORDS(parent, median, median+1, BT_COUNT(parent) - median - 1);

    /* shift the child pointers after right child to left by one */
    BT_MOVECHILDREN(parent, median+1, median+2, BT_COUNT(parent) - median - 1);
  }

  BT_COUNT(parent) = BT_COUNT(parent) - 1;
  /* -----------------------------------------------------------------*/

  /* save the changes */
  parent->children[median] = parent->T->WriteNode(left);
  parent->position = parent->T->WriteNode(parent);
  parent->T->DeleteNode(right);
  free(right->data);
  free(right);
}

/*
 * Internal recursive B-tree deletion function
 */
int BtreeDeleteRecursive(const byte* key, BtreeNode* node)
{
  BtreeNode* left = NULL;
  BtreeNode* right = NULL;
  BtreeNode* next = NULL;
  uint16 i = 0;
  int result = 0;

  while (i < BT_COUNT(node) && BT_KEYCMP(key, > ,BT_KEY(node,i), node))
  {
    i++;
  }

  /* key found? */
  if (i < BT_COUNT(node) && BT_KEYCMP(key, == ,BT_KEY(node,i), node))
  {
    /* if current node is a leaf, simply remove the record */
    if (BT_LEAF(node))
    {
      /* if there are records to be shifted */
      if ((BT_COUNT(node) - i - 1) > 0)
      {
        /* shift the records after the i-th record to left by one */
        BT_MOVERECORDS(node, i, i+1, BT_COUNT(node) - i - 1);
      }
      BT_COUNT(node) = BT_COUNT(node) - 1;
      node->position = node->T->WriteNode(node);
    }
    /* otherwise remove the record without destroying the tree's balance */
    else
    {
      /*
       * If possible (left child node before the key to be deleted
       * contains at least T keys), replace the key to be deleted by
       * its PREDECESSOR
       */
      left = node->T->ReadNode(node->children[i], node->T);

      if (BT_COUNT(left) >= BT_ORDER(node))
      {
        BT_COPYPREDECESSOR(node, i, left);
        node->position = node->T->WriteNode(node);

        result = BtreeDeleteRecursive(BT_KEY(left, BT_COUNT(left) - 1), left);

        free(left->data);
        free(left);

        return result;
      }

      /*
       * Otherwise, if possible (right child node before the key to be
       * deleted contains at least T keys), replace the key to be deleted
       * by its SUCCESSOR
       */
      right = node->T->ReadNode(node->children[i + 1], node->T);

      if (BT_COUNT(right) >= BT_ORDER(node))
      {
        BT_COPYSUCCESSOR(node, i, right);
        node->position = node->T->WriteNode(node);

        result = BtreeDeleteRecursive(right->records + BT_KEYPOS(right), right);

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
    return BTREE_DELETE_SUCCESS;
  }
  else
  {
    /* if current node is leaf, the record does not exist */
    if (BT_LEAF(node))
    {
      return BTREE_DELETE_NOTFOUND;
    }
    /* otherwise, re-balance the tree if necessary and recurse to subtree */
    else
    {
      next = node->T->ReadNode(node->children[i], node->T);

      /* if new subtree has only T - 1 keys, re-balance the tree */
      if (BT_COUNT(next) < BT_ORDER(node))
      {

        left = right = NULL;

        /* if a left sibling of the subtree root node
         * exists and has a key to spare...
         */
        if (i > 0)
        {
          left = node->T->ReadNode(node->children[i - 1], node->T);

          if (BT_COUNT(left) >= BT_ORDER(node))
          {
            /* ---------------------------------------------------------- *
             * perform a rotate operation from the left sibling,          *
             * through the parent node, into the subtree root node        *
             * ---------------------------------------------------------- */

            /* create space for the spared key/record */
            BT_MOVERECORDS(next, 1, 0, BT_COUNT(next));

            /* insert the current record into the subtree root node,
             * and replace the current record by its predecessor
             */
            BT_COPYRECORDS(next, 0, node, i - 1, 1);
            BT_COPYPREDECESSOR(node, i - 1, left);

            /* if the children are internal nodes, move the left
             * sibling's last child pointer to the subtree root node
             */
            if (BT_INTERNAL(next))
            {
              BT_MOVECHILDREN(next, 1, 0, BT_COUNT(next) + 1);
              next->children[0] = left->children[BT_COUNT(left)];
            }

            /* update all nodes */
            BT_COUNT(next) = BT_COUNT(next) + 1;
            BT_COUNT(left) = BT_COUNT(left) - 1;

            node->children[i - 1] = node->T->WriteNode(left);
            node->children[i] = node->T->WriteNode(next);
            node->position = node->T->WriteNode(node);

            free(left->data);
            free(left);
            /* ---------------------------------------------------------- */
            goto BTREE_DELETE_RECURSE;
          }
        }

        /* otherwise, if the right sibling of the
         * subtree root has a key to spare...
         */
        if (i < BT_COUNT(node))
        {
          right = node->T->ReadNode(node->children[i + 1], node->T);

          if (BT_COUNT(right) >= BT_ORDER(node))
          {
            /* ---------------------------------------------------------- *
             * perform a reverse rotate operation from the right sibling, *
             * through the parent node, into the subtree root node        *
             * ---------------------------------------------------------- */

            /* insert the current record into the subtree root node */
            BT_COPYRECORDS(next, BT_COUNT(next), node, i, 1);

            /* replace the current record by its successor */
            BT_COPYSUCCESSOR(node, i, right);

            /* if the children are internal nodes, move the right
             * sibling's first child pointer to the subtree root node
             */
            if (BT_INTERNAL(next))
            {
              next->children[BT_COUNT(next) + 1] = right->children[0];
              BT_MOVECHILDREN(right, 0, 1, BT_COUNT(right));
            }

            /* shift all records of the right sibling to left by one */
            BT_MOVERECORDS(right, 0, 1, BT_COUNT(right) - 1);

            /* update all nodes */
            BT_COUNT(next) = BT_COUNT(next) + 1;
            BT_COUNT(right) = BT_COUNT(right) - 1;

            node->children[i + 1] = node->T->WriteNode(right);
            node->children[i] = node->T->WriteNode(next);
            node->position = node->T->WriteNode(node);

            if (left != NULL)
            {
              free(left->data);
              free(left);
            }
            free(right->data);
            free(right);
            /* ---------------------------------------------------------- */
            goto BTREE_DELETE_RECURSE;
          }
        }

        /* otherwise, both the left and right siblings of the subtree
         * root node contain T - 1 keys, so perform a merge where possible
         */
        if (left != NULL)
        {
          BtreeMergeNodes(left, next, node, i-1);
          next = left;
        }
        else
        {
          BtreeMergeNodes(next, right, node, i);
        }
      }

      BTREE_DELETE_RECURSE:

      result = BtreeDeleteRecursive(key, next);
      return result;
    }
  }
}

/*
 * B-tree deletion function
 */
int BtreeDelete(const byte* key, Btree* t)
{
  BtreeNode *newRoot = NULL;

  if (t->root != NULL)
  {
    /* root node is empty and an internal node */
    if (BT_COUNT(t->root) == 0)
    {
      if (BT_INTERNAL(t->root))
      {
        /* the B-tree height decreases by one and the root node's
         * only child becomes the new root node
         */
        newRoot = t->ReadNode(t->root->children[0], t);
        t->DeleteNode(t->root);
        free(t->root->data);
        free(t->root);
        t->root = newRoot;
        /* continue deletion from new root node */
        return BtreeDeleteRecursive(key, t->root);
      }
      else
      {
        return BTREE_DELETE_EMPTYROOT;
      }
    }
    return BtreeDeleteRecursive(key, t->root);
  }
  else
  {
    return BTREE_DELETE_NOROOT;
  }
}
