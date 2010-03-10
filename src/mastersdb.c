
#include <stdio.h>
#include <malloc.h>

#include "libloader.h"
#include "btree.h"

#define BTREE_T             2
#define BTREE_RECORD_SIZE   1
#define BTREE_KEY_SIZE      1
#define BTREE_KEY_POSITION  0

  /*| COUNT | LEAF | CHILD0 | CHILD1 | CHILD2 | CHILD3 | REC0 | REC1 | REC1 |*/
/*
byte node_data[] = {
       2,0,   0,0,   1,0,0,0, 2,0,0,0, 3,0,0,0, 0,0,0,0, 'D',   'H',    0,
       3,0,   1,0,   0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 'A',   'B',   'C',
       3,0,   1,0,   0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 'E',   'F',   'G',
       3,0,   1,0,   0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 'I',   'J',   'K',
};
*/

byte node_data[] = {
       0,0,   1,0,   0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,   0,    0,
};


BtreeNode* ReadNode(const ulong position, const void* t) {
  const Btree* tree = (Btree*)t;
  ulong node_size = 2 * tree->meta.t * (tree->meta.record_size + 4) - tree->meta.record_size + 4;
  BtreeNode* node = BtreeCreateNode(tree);
  memcpy(node->data,node_data + position * node_size, node_size);
  return node;
}

void PrintNode(const BtreeNode* node, const Btree* tree) {
    uint16 i = 0;
    printf("%u ", *node->record_count);
    printf("%u ", *node->is_leaf);
    for (i = 0; i < tree->meta.t * 2; i++) {
      printf("%lu ", node->children[i]);
    }
    for (i = 0; i < (tree->meta.t * 2 - 1); i++) {
      printf("%c ", node->records[i]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
  byte* record = NULL;
  byte* keys = "ABCDEFGHIJKL\0";
  byte* key = keys;
  int i = 0;
  BtreeNode* node;

  Btree* t = BtreeCreateTree(BTREE_T,BTREE_RECORD_SIZE,BTREE_KEY_SIZE,BTREE_KEY_POSITION);

  t->CompareKeys = &memcmp;
  t->LoadNode = &ReadNode;
  t->root = ReadNode(0,t);

/*
  for (i=0; i<4; i++) {
    node = ReadNode(i,t);
    PrintNode(node,t);
    free (node->data);
    free (node);
  }
*/
/*
  do {
    record = BtreeSearch(key,t);
    printf("Record %c %s!\n", *key, (record != NULL) ? "found" : "not found");
  } while(*++key != '\0');
*/

  PrintNode(t->root,t);
  BtreeInsert("C",t);
  PrintNode(t->root,t);
  BtreeInsert("A",t);
  PrintNode(t->root,t);
  BtreeInsert("B",t);
  PrintNode(t->root,t);

  free(t);
	return 0;
}
