#include <stdio.h>
#include <malloc.h>

#include "mastersdb.h"

#define BTREE_T             5
#define BTREE_RECORD_SIZE   1
#define BTREE_KEY_SIZE      1
#define BTREE_KEY_POSITION  0

byte* node_data = NULL;
uint16 nodeCount = 0;

mdbBtreeNode* ReadNode(const uint32 position, mdbBtree* tree)
{
  uint32 node_size = (2 * tree->meta.order + 2) * sizeof(uint32) +
      (2 * tree->meta.order - 1) * tree->meta.record_size;

  mdbBtreeNode* node;
  mdbBtreeAllocateNode(&node, tree);
  memcpy(node->data, node_data + position * node_size, node_size);
  node->position = position;
  return node;
}

uint32 WriteNode(mdbBtreeNode* node)
{
  if (node->position > 0L)
  {
    memcpy(node_data + node->position * node->T->nodeSize, node->data,
        node->T->nodeSize);
  }
  else
  {
    node->position = ++nodeCount;
    node_data = realloc(node_data, (nodeCount + 1) * node->T->nodeSize);
    memcpy(node_data + node->position * node->T->nodeSize, node->data,
        node->T->nodeSize);
  }
  return node->position;
}

void DeleteNode(mdbBtreeNode* node)
{
  memset(node->data, 0, node->T->nodeSize);
  node->position = WriteNode(node);
}

void PrintNode(const mdbBtreeNode* node, const mdbBtree* tree)
{
  uint32 i = 0;
  printf("P(%2lu) ", (unsigned long)node->position);
  printf("R(%2lu) ", (unsigned long)(*node->record_count));
  printf("L(%2lu) [", (unsigned long)(*node->is_leaf));
  for (i = 0; i < tree->meta.order * 2; i++)
  {
    if (i > *node->record_count || *node->is_leaf > 0)
    {
      printf(".. ");
    }
    else
    {
      printf("%2lu ", (unsigned long)node->children[i]);
    }
  }
  printf("] ");
  for (i = 0; i < (tree->meta.order * 2 - 1); i++)
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

void PrintTypeTable(mdbDatatype *typetable) {
  byte i;

  printf("-----------------------------\n");
  for (i=0; i<MDB_TYPE_COUNT; i++) {
    printf("  %12s %2u %2u %2u %p\n",
        typetable[i].name, typetable[i].length, typetable[i].header,
        typetable[i].size, typetable[i].compare);
  }
  printf("-----------------------------\n");
}

int main(int argc, char **argv)
{
//  uint32 i = 0;
//  char* buffer = (char*) malloc(128);
//  char* searchResult = NULL;
//  mdbBtreeNode* node;
//  mdbBtree* t = NULL;
//  mdbDatatype *types = NULL;//, *type = NULL;
//
//  mdbInitializeTypes(&types);
//
//  mdbBtreeCreateTree(&t, BTREE_T, BTREE_RECORD_SIZE, BTREE_KEY_SIZE,
//      BTREE_KEY_POSITION);
//
//  char *insert = "ABCDEFGHIJKLMNOPQRSTUVXYZ\0";
//  char *delete = "\0ZYXVUTSRQPONMLKJIHGFEDCB\0";
//
//  t->CompareKeys = &memcmp;
//  t->ReadNode = &ReadNode;
//  t->WriteNode = &WriteNode;
//  t->DeleteNode = &DeleteNode;
//
//  nodeCount = 1;
//  node_data = calloc(nodeCount + 1, t->nodeSize);
//
//  t->root = ReadNode(1, t);
//  *(((mdbBtreeNode*) t->root)->is_leaf) = 1;
//
//  while (*insert != '\0')
//  {
//    mdbBtreeInsert(insert++, t);
//  }
//
//  while (*delete != '\0')
//  {
//    mdbBtreeDelete(delete++, t);
//  }
//
//  do
//  {
//    printf(
//        "Enter command [" "(i)nsert," "(d)elete," "(s)earch," "(p)rint,"
//        "(t)ype table," "(q)uit,(t)] >> ");
//    fgets(buffer, 128, stdin);
//    switch (buffer[0])
//    {
//      case 'i':
//        printf("*** INSERT - enter record: ");
//        fgets(buffer, 128, stdin);
//        i = mdbBtreeInsert(&buffer[0], t);
//        printf("*** RESULT: %s\n", (i == MDB_BTREE_INSERT_SUCCESS) ? "SUCCESS"
//            : "COLLISION!");
//        buffer[0] = 'i';
//        break;
//      case 'p':
//        printf("*** PRINT\n");
//        printf("--------------------------\n");
//        for (i = 1; i <= nodeCount; i++)
//        {
//          node = ReadNode(i, t);
//          PrintNode(node, t);
//          free(node->data);
//          free(node);
//        }
//        printf("--------------------------\n");
//        break;
//      case 'd':
//        printf("*** DELETE - enter record: ");
//        fgets(buffer, 128, stdin);
//        i = mdbBtreeDelete(&buffer[0], t);
//        switch (i)
//        {
//          case MDB_BTREE_DELETE_NOROOT:
//            printf("*** RESULT: %s\n", "NO ROOT!");
//            break;
//          case MDB_BTREE_DELETE_EMPTYROOT:
//            printf("*** RESULT: %s\n", "EMPTY ROOT!!");
//            break;
//          case MDB_BTREE_DELETE_NOTFOUND:
//            printf("*** RESULT: %s\n", "RECORD NOT FOUND!");
//            break;
//          case MDB_BTREE_DELETE_SUCCESS:
//            printf("*** RESULT: %s\n", "SUCCESS");
//            break;
//          default:
//            printf("*** RESULT: UNKNOWN(%d)\n", i);
//            break;
//        }
//        buffer[0] = 'd';
//        break;
//      case 's':
//        printf("*** SEARCH - enter record: ");
//        fgets(buffer, 128, stdin);
//        i = mdbBtreeSearch(&buffer[0], &searchResult, t);
//        if (searchResult != NULL)
//        {
//          printf("*** FOUND! (%c)\n", *searchResult);
//          free(searchResult);
//          searchResult = NULL;
//        }
//        else
//        {
//          printf("*** NOT FOUND!\n");
//        }
//        buffer[0] = 's';
//        break;
//      case 't':
//        printf("*** TYPE TABLE\n");
//        PrintTypeTable(types);
//        break;
//      case 'q':
//        printf("*** QUIT\n");
//        break;
//      default:
//        printf("*** Ignored!\n");
//        break;
//    }
//  } while (buffer[0] != 'q');
//
//  free(t);
//  return 0;

  mdbDatabase *db;

//  return mdbCreateDatabase(&db, "test.mrdb");
  return mdbOpenDatabase(&db, "test.mrdb");
}
