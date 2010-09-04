/*
 * mdbbtree.c
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
 * 19.07.2010
 *  Updating the types to be uint32 or char* (before: uint16 and byte*).
 * 06.08.2010
 *  Added the BtreeTraverse function.
 * 07.08.2010
 *  Removed "key_size" from mdbBtreeMeta.
 * 09.08.2010
 *  Added mdbBtreeOptimalOrder function.
 */

#include "mdb.h"
#include "mdbbtree_util.h"

mdbBtreeNode* mdbReadNode(const uint32 position, mdbBtree* tree)
{
  mdbBtreeNode *node;
  mdbAllocateNode(&node, tree);
  node->position = position;
  fseek(tree->file, position, SEEK_SET);
  fread(node->data, tree->nodeSize, 1, tree->file);
  return node;
}

uint32 mdbWriteNode(mdbBtreeNode* node)
{
  if (node->position > 0)
  {
    fseek(node->T->file, node->position, SEEK_SET);
  }
  else
  {
    fseek(node->T->file, 0, SEEK_END);
    node->position = ftell(node->T->file);
  }
  fwrite(node->data, node->T->nodeSize, 1, node->T->file);
  return node->position;
}

void mdbDeleteNode(mdbBtreeNode* node)
{

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
mdbError mdbAllocateNode(mdbBtreeNode** node, mdbBtree *tree)
{
  *node = (mdbBtreeNode*) malloc(sizeof(mdbBtreeNode));

  /* Allocates the memory */
  (*node)->data = (char*) malloc(tree->nodeSize);
  memset((*node)->data, 0, tree->nodeSize);

  /* Initializes the data pointers */
  (*node)->record_count = (uint32*)(*node)->data;
  (*node)->is_leaf = (*node)->record_count + 1;
  (*node)->children = (uint32*)((*node)->is_leaf + 1);
  (*node)->records = (char*)((*node)->children + (tree->meta.order << 1));
  (*node)->T = tree;
  (*node)->position = 0L;


  return MDB_NO_ERROR;
}

/* Frees up the memory used by a B-tree node (with eventual save) */
mdbError mdbFreeNode(mdbBtreeNode* node, uint8 save)
{
  if (save > 0)
  {
    node->position = mdbWriteNode(node);
  }
  free (node->data);
  free (node);

  return MDB_NO_ERROR;
}

/* B-tree key comparison based on the data type of the key */
int mdbBtreeCmp(const char* k1, const char* k2, const mdbBtree *tree)
{
  uint32 size1, size2;
  if (tree->key_type->header > 0)
  {
    size1 = *((uint32*)k1);
    size2 = *((uint32*)k2);
    if (size2 < size1) size1 = size2;
    return tree->key_type->compare(
        k1 + tree->key_type->header, k2 + tree->key_type->header, size1);
  }
  return tree->key_type->compare(k1, k2, tree->key_type->size);
}

/* Calculates the optimal order of a B-tree for the given record size */
uint32 mdbBtreeOptimalOrder(uint32 record_size)
{
  const static uint32 LOWER_NS = 1<<10; /* lowest possible node size: 1 KB*/
  const static uint32 UPPER_NS = 1<<20; /* highest possible node size:1 MB*/
  uint32 ideal_ns;                      /* ideal node size                  */
  uint32 real_ns;                       /* real node size                   */
  uint32 opt_order = 0L;                /* optimal B-tree order             */
  uint32 cur_order;                     /* current calculated B-tree order  */
  int cur_diff;                         /* current (ideal - real) difference*/
  int min_diff = UPPER_NS;              /* minimum (ideal - real) difference*/

  for (ideal_ns = LOWER_NS; ideal_ns <= UPPER_NS; ideal_ns += 1024)
  {
    if (ideal_ns < record_size) continue;

    cur_order = (ideal_ns + record_size - 8) / ((record_size + 4) * 2);
    if (opt_order == 0L) opt_order = cur_order;
    real_ns = (cur_order + 1) * 8 + ((cur_order * 2) - 1) * record_size;
    cur_diff = (int)ideal_ns - (int)real_ns;

    if (cur_diff <= min_diff)
    {
      min_diff = cur_diff;
      opt_order = cur_order;
    }
  }
  return opt_order;
}

/*
 * B-tree allocation and initialization
 */
mdbError mdbBtreeCreate(mdbBtree** tree,
    uint32 order,
    const uint32 record_size,
    const uint32 key_position)
{
  /* allocates a B-tree structure */
  *tree = (mdbBtree*) malloc(sizeof(mdbBtree));

  /* initializes the B-tree structure */
  if (order == 0L) {
    order = mdbBtreeOptimalOrder(record_size);
  }
  (*tree)->meta.order = order;
  (*tree)->meta.record_size = record_size;
  (*tree)->meta.key_position = key_position;
  (*tree)->meta.root_position = 0L;

  /* default node functions */
  (*tree)->ReadNode = &mdbReadNode;
  (*tree)->WriteNode = &mdbWriteNode;
  (*tree)->DeleteNode = &mdbDeleteNode;

  BT_CALC_NODESIZE(*tree);

  return MDB_NO_ERROR;
}

/*
 * Recursive B-tree search (internal, not visible to the programmer)
 */
mdbError mdbBtreeSearchRecursive(
    const char* key,
    char* record,
    mdbBtreeNode* node)
{
  mdbBtreeNode* next = NULL;
  int result = 0;
  uint32 i = 0;

  while (i < BT_COUNT(node) && (mdbBtreeCmp(key,BT_KEY(node,i),node->T) > 0))
  {
    i++;
  }

  /* key/record found? */
  if (i < BT_COUNT(node))
  {
    if (mdbBtreeCmp(key,BT_KEY(node,i),node->T) == 0)
    {
      memcpy(record, BT_RECORD(node,i), BT_RECSIZE(node));
      return MDB_NO_ERROR;
    }
  }

  /* if node is a leaf, search for the key and return NULL if not found */
  if (BT_LEAF(node))
  {
    result = MDB_BTREE_KEY_NOT_FOUND;
  }
  /* if node is an internal node, search for the key or recurse to subtree */
  else
  {
    next = node->T->ReadNode(node->children[i], node->T);
    result = mdbBtreeSearchRecursive(key, record, next);
    free(next->data);
    free(next);
  }

  return result;
}

/*
 * B-tree search function
 */
mdbError mdbBtreeSearch(const char* key, char* record, mdbBtree* t)
{
  if (t->root != NULL)
  {
    return mdbBtreeSearchRecursive(key, record, t->root);
  }
  else
  {
    return MDB_BTREE_NO_ROOT;
  }
}

/*
 * Internal function for splitting a full node
 */
void mdbBtreeSplitNode(mdbBtreeNode* left, mdbBtreeNode* parent,
    const uint32 position)
{
  mdbBtreeNode* right = NULL;
  mdbAllocateNode(&right, left->T);

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
mdbError mdbBtreeInsertRecursive(const char* record, mdbBtreeNode* node)
{
  mdbBtreeNode* next = NULL;
  uint32 i = 0;
  int result = 0;

  while (i < BT_COUNT(node) &&
      (mdbBtreeCmp(record + BT_KEYPOS(node),BT_KEY(node,i),node->T) > 0))
  {
    i++;
  }

  /* key already exists? */
  if (i < BT_COUNT(node))
  {
    if (mdbBtreeCmp(record + BT_KEYPOS(node),BT_KEY(node,i),node->T) == 0)
    {
      return MDB_BTREE_KEY_COLLISION;
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
    return MDB_NO_ERROR;
  }

  /* if node is an internal node recurse to the subtree */
  else
  {
    next = node->T->ReadNode(node->children[i], node->T);

    /* child node is full, split it */
    if (BT_COUNT(next) == (BT_ORDER(node) << 1) - 1)
    {
      mdbBtreeSplitNode(next, node, i);

      /* the current key changed and the current subtree maybe smaller than
       * then the record's key, so determine the direction of the recursion
       * (left or right) */
      if (mdbBtreeCmp(record + BT_KEYPOS(node),BT_KEY(node,i),node->T) > 0)
      {
        free(next->data);
        free(next);
        next = node->T->ReadNode(node->children[i + 1], node->T);
      }
      else if (mdbBtreeCmp(record+BT_KEYPOS(node),BT_KEY(node,i),node->T) == 0)
      {
        free(next->data);
        free(next);
        return MDB_BTREE_KEY_COLLISION;
      }
    }

    result = mdbBtreeInsertRecursive(record, next);
    free(next->data);
    free(next);
    return result;
  }
}

/*
 * B-tree insertion function
 */
mdbError mdbBtreeInsert(const char* record, mdbBtree* t)
{
  mdbBtreeNode* newRoot = NULL;

  if (t->root != NULL)
  {
    /* t->root node is full, split it */
    if (BT_COUNT(t->root) == (BT_ORDER(t->root) << 1) - 1)
    {
      mdbAllocateNode(&newRoot, t);

      /* the new t->root will be an internal node, while the old t->root
       * will become a leaf node and get a right sibling
       */
      newRoot->position = 0L;
      *newRoot->is_leaf = 0;

      mdbBtreeSplitNode(t->root, newRoot, 0);

      /* free up used resources */
      free(t->root->data);
      free(t->root);
      t->root = newRoot;
    }
    return mdbBtreeInsertRecursive(record, t->root);
  }
  else
  {
    return MDB_BTREE_NO_ROOT;
  }
}

/*
 * Internal function for merging two child nodes with median from parent
 */
void mdbBtreeMergeNodes(mdbBtreeNode* left, mdbBtreeNode* right,
    mdbBtreeNode* parent, const uint32 median)
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
mdbError mdbBtreeDeleteRecursive(const char* key, mdbBtreeNode* node)
{
  mdbBtreeNode* left = NULL;
  mdbBtreeNode* right = NULL;
  mdbBtreeNode* next = NULL;
  uint32 i = 0;
  int result = 0;

  while (i < BT_COUNT(node) && (mdbBtreeCmp(key,BT_KEY(node,i),node->T) > 0))
  {
    i++;
  }

  /* key found? */
  if (i < BT_COUNT(node) && (mdbBtreeCmp(key,BT_KEY(node,i),node->T) == 0))
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

        result
            = mdbBtreeDeleteRecursive(BT_KEY(left, BT_COUNT(left) - 1), left);

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

        result = mdbBtreeDeleteRecursive(right->records + BT_KEYPOS(right),
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
      mdbBtreeMergeNodes(left, right, node, i);

      result = mdbBtreeDeleteRecursive(key, left);
      free(left->data);
      free(left);
      return result;
    }
    return MDB_NO_ERROR;
  }
  else
  {
    /* if current node is leaf, the record does not exist */
    if (BT_LEAF(node))
    {
      return MDB_BTREE_KEY_NOT_FOUND;
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
            goto MDB_BTREE_DELETE_RECURSE;
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
            goto MDB_BTREE_DELETE_RECURSE;
          }
        }

        /* otherwise, both the left and right siblings of the subtree
         * root node contain T - 1 keys, so perform a merge where possible
         */
        if (left != NULL)
        {
          mdbBtreeMergeNodes(left, next, node, i-1);
          next = left;
        }
        else
        {
          mdbBtreeMergeNodes(next, right, node, i);
        }
      }

      MDB_BTREE_DELETE_RECURSE:

      result = mdbBtreeDeleteRecursive(key, next);
      return result;
    }
  }
}

/*
 * B-tree deletion function
 */
mdbError mdbBtreeDelete(const char* key, mdbBtree* t)
{
  mdbBtreeNode *newRoot = NULL;

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
        return mdbBtreeDeleteRecursive(key, t->root);
      }
      else
      {
        return MDB_BTREE_ROOT_IS_EMPTY;
      }
    }
    return mdbBtreeDeleteRecursive(key, t->root);
  }
  else
  {
    return MDB_BTREE_NO_ROOT;
  }
}

mdbError mdbBtreeTraverse(mdbBtreeTraversal **t, char *record)
{
  mdbBtreeTraversal *tmp;
  mdbBtree *tree = (*t)->node->T;

  /* find left-most leaf node */
  while (BT_INTERNAL((*t)->node))
  {
    tmp = (mdbBtreeTraversal*)malloc(sizeof(mdbBtreeTraversal));
    tmp->parent = *t;
    tmp->position = 0;
    tmp->node = tree->ReadNode((*t)->node->children[(*t)->position], tree);
    *t = tmp;
  }

  /* if there are still records to be returned from current leaf... */
  if ((*t)->position < *(*t)->node->record_count)
  {
    memcpy(record,BT_RECORD((*t)->node,(*t)->position),BT_RECSIZE((*t)->node));
    (*t)->position++;
  }
  /* else, go back to parent and return the parent record */
  else
  {
    do
    {
      if ((*t)->parent == NULL)
      {
        /* no more records */
        return MDB_BTREE_NO_MORE_RECORDS;
      }
      tmp = (*t)->parent;
      free((*t)->node->data);
      free(*t);
      *t = tmp;
    }
    while (tmp->position == *tmp->node->record_count);

    memcpy(record,BT_RECORD((*t)->node,(*t)->position),BT_RECSIZE((*t)->node));
    (*t)->position++;

    /* load the right child */
    if (BT_INTERNAL((*t)->node))
    {
      tmp = (mdbBtreeTraversal*)malloc(sizeof(mdbBtreeTraversal));
      tmp->parent = *t;
      tmp->position = 0;
      tmp->node = tree->ReadNode((*t)->node->children[(*t)->position], tree);
      *t = tmp;
    }
  }

  return MDB_NO_ERROR;
}
