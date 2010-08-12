
//#include "mdbql/Parser.h"
//
//int main(int argc, char **argv)
//{
//  mdbDatabase *db;
//  mdbTable *tbl;
//  Scanner *s;
//  Parser *p;
//
//  MastersDBVM *VM;
//  int ret;
//
//  const char *query =
//      "CREATE TABLE Studenti(\n"
//         "Ime STRING(20),"
//         "Prezime STRING(50)"
//      ");";
//
//  s = new Scanner((byte*)query, strlen(query));
//  p = new Parser(s);
//
//  // creates a new MastersDB database
//  ret = mdbCreateDatabase(&db, "test.mrdb");
//  VM = new MastersDBVM(db);
//  p->setVM(VM);
//
//  // parses and executes the MQL query
//  p->Parse();
//  VM->Execute();
//
//  ret = mdbCloseDatabase(db);
//
//  delete s;
//  delete p;
//  delete VM;
//
//  ret = mdbOpenDatabase(&db, "test.mrdb");
//  ret = mdbLoadTable(db, &tbl, "Studenti");
//  ret = mdbFreeTable(tbl);
//  ret = mdbCloseDatabase(db);
//
//  return 0;
//}

extern "C" {
  #include "mastersdb.h"
}

#include "mdbvm/MastersDBVM.h"

using namespace MDB;

int main(int argc, char **argv)
{

  mdbDatabase *db;
  mdbTable *tbl;
  MastersDBVM *VM;
  int ret;

  // Table meta data

  char *name = new char[12];
  strcpy(name + 4, "STUDENTI");
  *((uint32*)name) = strlen("STUDENTI");

  mdbColumn *col1 = new mdbColumn;
  mdbColumn *col2 = new mdbColumn;

  strcpy(col1->name + 4, "IME");
  *((uint32*)col1->name) = strlen("IME");
  col1->indexed = 0;
  col1->length = 20L;
  col1->type = 4;

  strcpy(col2->name + 4, "PREZIME");
  *((uint32*)col2->name) = strlen("PREZIME");
  col2->indexed = 0;
  col2->length = 30L;
  col2->type = 4;

  // DB and VM operations

  ret = mdbCreateDatabase(&db, "test.mrdb");

  VM = new MastersDBVM(db);
  VM->Store(name, 0);
  VM->Store((char*)col1, 1);
  VM->Store((char*)col2, 2);

  VM->AddInstruction(MastersDBVM::PUSH, 2);
  VM->AddInstruction(MastersDBVM::ADDTBL, 0);
  VM->AddInstruction(MastersDBVM::ADDCOL, 1);
  VM->AddInstruction(MastersDBVM::ADDCOL, 2);
  VM->AddInstruction(MastersDBVM::CRTBL, 0);

  VM->Decode();
  VM->Decode();
  VM->Decode();
  VM->Decode();
  VM->Decode();

  delete VM;

  ret = mdbCloseDatabase(db);

  ret = mdbOpenDatabase(&db, "test.mrdb");
  ret = mdbLoadTable(db, &tbl, "STUDENTI");
  ret = mdbFreeTable(tbl);
  ret = mdbCloseDatabase(db);


  return 0;
}

//#define BTREE_T             5
//#define BTREE_RECORD_SIZE   5
//#define BTREE_KEY_POSITION  0
//
//byte* node_data = NULL;
//uint16 nodeCount = 0;
//
//mdbBtreeNode* ReadNode(const uint32 position, mdbBtree* tree)
//{
//  uint32 node_size = (2 * tree->meta.order + 2) * sizeof(uint32) +
//      (2 * tree->meta.order - 1) * tree->meta.record_size;
//
//  mdbBtreeNode* node;
//  mdbBtreeAllocateNode(&node, tree);
//  memcpy(node->data, node_data + position * node_size, node_size);
//  node->position = position;
//  return node;
//}
//
//uint32 WriteNode(mdbBtreeNode* node)
//{
//  if (node->position > 0L)
//  {
//    memcpy(node_data + node->position * node->T->nodeSize, node->data,
//        node->T->nodeSize);
//  }
//  else
//  {
//    node->position = ++nodeCount;
//    node_data = (byte*)realloc(node_data, (nodeCount + 1) * node->T->nodeSize);
//    memcpy(node_data + node->position * node->T->nodeSize, node->data,
//        node->T->nodeSize);
//  }
//  return node->position;
//}
//
//void DeleteNode(mdbBtreeNode* node)
//{
//  memset(node->data, 0, node->T->nodeSize);
//  node->position = WriteNode(node);
//}
//
//void PrintNode(const mdbBtreeNode* node, const mdbBtree* tree)
//{
//  uint32 i = 0;
//  printf("P(%2lu) ", (unsigned long)node->position);
//  printf("R(%2lu) ", (unsigned long)(*node->record_count));
//  printf("L(%2lu) [", (unsigned long)(*node->is_leaf));
//  for (i = 0; i < tree->meta.order * 2; i++)
//  {
//    if (i > *node->record_count || *node->is_leaf > 0)
//    {
//      printf(".. ");
//    }
//    else
//    {
//      printf("%2lu ", (unsigned long)node->children[i]);
//    }
//  }
//  printf("] ");
//  for (i = 0; i < (tree->meta.order * 2 - 1); i++)
//  {
//    if (i < *node->record_count)
//    {
//      printf("%c ", node->records[i * BTREE_RECORD_SIZE + 4]);
//    }
//    else
//    {
//      printf("_ ");
//    }
//  }
//  printf("\n");
//}
//
//void PrintTypeTable(mdbDatatype *typetable) {
//  byte i;
//
//  printf("-----------------------------\n");
//  for (i=0; i<MDB_TYPE_COUNT; i++) {
//    printf("  %12s %2u %2u %2u %p\n",
//        typetable[i].name, typetable[i].length, typetable[i].header,
//        typetable[i].size, typetable[i].compare);
//  }
//  printf("-----------------------------\n");
//}
//
//int main(int argc, char **argv)
//{
//  uint32 i = 0;
//  char* buffer = (char*) malloc(128);
//  char searchResult[5];
//  mdbBtreeNode* node;
//  mdbBtree* t = NULL;
//  mdbDatatype mdbString = { "STRING",6,4, sizeof(byte),
//    (CompareKeysPtr)&strncmp};
//
//  mdbBtreeTraversal *trv = (mdbBtreeTraversal*)malloc(sizeof(mdbBtreeTraversal));
//
//  mdbBtreeCreateTree(&t, BTREE_T, BTREE_RECORD_SIZE, BTREE_KEY_POSITION);
//
//  const char *ins = "ABCDEFGHIJKLMNOPQRSTUVXYZ\0";
//  const char *del = "\0ZYXVUTSRQPONMLKJIHGFEDCB\0";
//
//  t->key_type = &mdbString;
//  t->ReadNode = &ReadNode;
//  t->WriteNode = &WriteNode;
//  t->DeleteNode = &DeleteNode;
//
//  nodeCount = 1;
//  node_data = (byte*)calloc(nodeCount + 1, t->nodeSize);
//
//  t->root = ReadNode(1, t);
//  *(((mdbBtreeNode*) t->root)->is_leaf) = 1;
//
//  while (*ins != '\0')
//  {
//    *((uint32*)&searchResult[0]) = 1;
//    searchResult[4] = *ins;
//    ins++;
//    mdbBtreeInsert(searchResult, t);
//  }
//
//  while (*del != '\0')
//  {
//    *((uint32*)&searchResult[0]) = 1;
//    searchResult[4] = *del;
//    del++;
//    mdbBtreeDelete(searchResult, t);
//  }
//
//  do
//  {
//    printf(
//        "Enter command [" "(i)nsert," "(d)elete," "(s)earch," "(p)rint,"
//        "(t)raverse," "(q)uit] >> ");
//    fgets(buffer, 128, stdin);
//    switch (buffer[0])
//    {
//      case 'i':
//        printf("*** INSERT - enter record: ");
//        fgets(buffer + 4, 128, stdin);
//        *((uint32*)buffer) = 1;
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
//        fgets(buffer + 4, 128, stdin);
//        *((uint32*)buffer) = 1;
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
//        fgets(buffer + 4, 128, stdin);
//        *((uint32*)buffer) = 1;
//        i = mdbBtreeSearch(&buffer[0], searchResult, t);
//        if (searchResult != NULL)
//        {
//          printf("*** FOUND! (%s)\n", searchResult);
//        }
//        else
//        {
//          printf("*** NOT FOUND!\n");
//        }
//        buffer[0] = 's';
//        break;
//      case 't':
//        printf("*** TRAVERSE\n");
//        trv->node = t->root;
//        trv->parent = NULL;
//        trv->position = 0;
//        while (mdbBtreeTraverse(&trv, &buffer[0]) != MDB_BTREE_NOTFOUND)
//        {
//          printf("%c\n", buffer[0]);
//        }
//        buffer[0] = 't';
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
//  }
