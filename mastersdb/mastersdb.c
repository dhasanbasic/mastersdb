#include <stdio.h>
#include <malloc.h>

#include "libloader.h"
#include "btree.h"

#define BTREE_T             3
#define BTREE_RECORD_SIZE   1
#define BTREE_KEY_SIZE      1
#define BTREE_KEY_POSITION  0

/*  | COUNT | LEAF | CHILD0 | CHILD1 | CHILD2 | CHILD3 | CHILD4 | CHILD5 | REC0 | REC1 | REC2 |*/

byte* node_data = NULL;
uint16 nodeCount = 0;

void* ReadNode(const ulong position, const void* t)
{
  Btree* tree = (Btree*) t;
  ulong node_size = 2 * tree->meta.t * (tree->meta.record_size + 4)
      - tree->meta.record_size + 4;
  BtreeNode* node = BtreeAllocateNode(tree);
  memcpy(node->data, node_data + position * node_size, node_size);
  node->position = position;
  return node;
}

ulong WriteNode(void* nodeptr, const void* t)
{
  Btree* tree = (Btree*) t;
  BtreeNode* node = nodeptr;
  ulong node_size = 2 * tree->meta.t * (tree->meta.record_size + 4)
      - tree->meta.record_size + 4;
  if (node->position > 0L)
  {
    memcpy(node_data + node->position * node_size, node->data, node_size);
  }
  else
  {
    node->position = ++nodeCount;
    node_data = realloc(node_data, (nodeCount + 1) * node_size);
    memcpy(node_data + node->position * node_size, node->data, node_size);
  }
  return node->position;
}

void DeleteNode(void* nodeptr, const void* t)
{
  Btree* tree = (Btree*) t;
  BtreeNode* node = nodeptr;
  memset(node->data, 0, tree->nodeSize);
  node->position = WriteNode(node, t);
}

void PrintNode(const BtreeNode* node, const Btree* tree)
{
  uint16 i = 0;
  printf("P(%2lu) ", node->position);
  printf("R(%u) ", *node->record_count);
  printf("L(%u) [", *node->is_leaf);
  for (i = 0; i < tree->meta.t * 2; i++)
  {
    if (i > *node->record_count || *node->is_leaf > 0)
    {
      printf(".. ");
    }
    else
    {
      printf("%2lu ", node->children[i]);
    }
  }
  printf("] ");
  for (i = 0; i < (tree->meta.t * 2 - 1); i++)
  {
    if (i < *node->record_count)
    {
      printf("%c ", node->records[i]);
    }
    else
    {
      printf("_ ");
    }
  }
  printf("\n");
}

int main(int argc, char **argv)
{
  int i = 0;
  byte* buffer = (byte*) malloc(128);
  byte* searchResult = NULL;
  BtreeNode* node;

  Btree* t = BtreeCreateTree(BTREE_T, BTREE_RECORD_SIZE, BTREE_KEY_SIZE,
      BTREE_KEY_POSITION);

  byte *records = "ABCDEFGJKLMNOPQRSTUVXYZ\0";

  t->CompareKeys = &memcmp;
  t->ReadNode = &ReadNode;
  t->WriteNode = &WriteNode;
  t->DeleteNode = &DeleteNode;

  nodeCount = 1;
  node_data = calloc(nodeCount + 1, t->nodeSize);

  t->root = ReadNode(1, t);
  *(((BtreeNode*) t->root)->is_leaf) = 1;

  while (*records != 'G')
  {
    BtreeInsert(records++, t);
  }

  do
  {
    printf("Enter command [(i)nsert,(d)elete,(s)earch,(p)rint,(q)uit] >> ");
    fgets(buffer, 128, stdin);
    switch (buffer[0])
    {
      case 'i':
        printf("*** INSERT - enter record: ");
        fgets(buffer, 128, stdin);
        i = BtreeInsert(&buffer[0], t);
        printf("*** RESULT: %s\n", (i == BTREE_INSERT_SUCCEEDED) ? "SUCCESS"
            : "COLLISION!");
        buffer[0] = 'i';
        break;
      case 'p':
        printf("*** PRINT\n");
        printf("--------------------------\n");
        for (i = 1; i <= nodeCount; i++)
        {
          node = ReadNode(i, t);
          PrintNode(node, t);
          free(node->data);
          free(node);
        }
        printf("--------------------------\n");
        break;
      case 'd':
        printf("*** DELETE - enter record: ");
        fgets(buffer, 128, stdin);
        i = BtreeDelete(&buffer[0], t);
        switch (i)
        {
          case BTREE_DELETE_NOROOT:
            printf("*** RESULT: %s\n", "NO ROOT!");
            break;
          case BTREE_DELETE_EMPTYROOT:
            printf("*** RESULT: %s\n", "EMPTY ROOT!!");
            break;
          case BTREE_DELETE_NOTFOUND:
            printf("*** RESULT: %s\n", "RECORD NOT FOUND!");
            break;
          case BTREE_DELETE_SUCCEEDED:
            printf("*** RESULT: %s\n", "SUCCESS");
            break;
          default:
            printf("*** RESULT: UNKNOWN(%d)\n", i);
            break;
        }
        buffer[0] = 'd';
        break;
      case 's':
        printf("*** SEARCH - enter record: ");
        fgets(buffer, 128, stdin);
        searchResult = BtreeSearch(&buffer[0], t);
        if (searchResult != NULL) {
          printf("*** FOUND! (%c)\n", *searchResult);
          free(searchResult);
          searchResult = NULL;
        } else {
          printf("*** NOT FOUND!\n");
        }
        buffer[0] = 's';
        break;
      case 'q':
        printf("*** QUIT\n");
        break;
      default:
        printf("*** Ignored!\n");
        break;
    }
  } while (buffer[0] != 'q');

  free(t);
  return 0;
}
