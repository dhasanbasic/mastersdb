
#include <iostream>
#include "MastersDB.h"

using namespace MastersDB;
using namespace std;

void printResults(MdbResultSet *rs)
{
  uint8_t c;
  if (rs != NULL)
  {
    cout << "Retrieved " << rs->GetRecordCount() << " records!" << endl;
    cout << endl;
    cout << rs->GetColumnName(0);
    for (c = 1; c < rs->GetColumnCount(); c++)
    {
      cout << "\t" << rs->GetColumnName(c);
    }
    do
    {
      cout << endl;

      if (rs->GetColumnType(0) == MDB_STRING)
      {
        cout << rs->GetStringValue(0);
      }
      else
      {
        cout << rs->GetIntValue(0);
      }
      for (c = 1; c < rs->GetColumnCount(); c++)
      {
        cout <<  "\t";
        if (rs->GetColumnType(c) == MDB_STRING)
        {
          cout << rs->GetStringValue(c);
        }
        else
        {
          cout << rs->GetIntValue(c);
        }
      }
    }
    while (rs->ToNext());
    cout << endl;
  }
}

int main(int argc, char **argv)
{
  MdbDatabase *db;
  MdbResultSet *rs;

//  db = MdbDatabase::CreateDatabase("test.mrdb");
//
//  cout << "=============================================" << endl;
//  cout << "              Creating tables                " << endl;
//  cout << "=============================================" << endl;
//
//  rs = db->ExecuteMQL("CREATE TABLE Odjeli (ID INT-8, Naziv STRING(20));");
//  rs = db->ExecuteMQL("CREATE TABLE Zaposleni (Ime STRING(20), Prezime STRING(40), Odjel INT-8);");
//
//  cout << "=============================================" << endl;
//  cout << "             Inserting records               " << endl;
//  cout << "=============================================" << endl;
//
//  rs = db->ExecuteMQL("INSERT INTO Odjeli VALUES (1,'System Testing');");
//  rs = db->ExecuteMQL("INSERT INTO Odjeli VALUES (2,'Development');");
//  rs = db->ExecuteMQL("INSERT INTO Odjeli VALUES (3,'Poduct Management');");
//
//  rs = db->ExecuteMQL("INSERT INTO Zaposleni VALUES ('Dinko','Hasanbašić', 2);");
//  rs = db->ExecuteMQL("INSERT INTO Zaposleni VALUES ('Jasmin','Velić', 1);");
//  rs = db->ExecuteMQL("INSERT INTO Zaposleni VALUES ('Amar','Trnka', 2);");
//  rs = db->ExecuteMQL("INSERT INTO Zaposleni VALUES ('Edin','Deljkić', 3);");
//  rs = db->ExecuteMQL("INSERT INTO Zaposleni VALUES ('Davor','Kovačić', 2);");
//  rs = db->ExecuteMQL("INSERT INTO Zaposleni VALUES ('Aida','Riković', 1);");
//  rs = db->ExecuteMQL("INSERT INTO Zaposleni VALUES ('Ferid','Ajanović', 3);");
//  rs = db->ExecuteMQL("INSERT INTO Zaposleni VALUES ('Adnan','Behram', 1);");
//
//  delete db;

  db = MdbDatabase::OpenDatabase("test.mrdb");

//  rs = db->ExecuteMQL("SELECT * FROM Zaposleni;");
//  printResults(rs);
//  delete rs;
//
//  rs = db->ExecuteMQL("SELECT Ime, Prezime FROM Zaposleni;");
//  printResults(rs);
//  delete rs;
//
//  rs = db->ExecuteMQL(
//      "SELECT Zaposleni.Ime, Zaposleni.Prezime, Odjeli.Naziv"
//      " FROM Zaposleni, Odjeli;");
//  printResults(rs);
//  delete rs;

//  rs = db->ExecuteMQL("SELECT * FROM Zaposleni WHERE Ime > 'C';");
//  printResults(rs);
//  delete rs;

//  rs = db->ExecuteMQL(
//  "SELECT Zaposleni.Ime, Zaposleni.Prezime, Odjeli.Naziv"
//  " FROM Zaposleni, Odjeli"
//  " WHERE Zaposleni.Odjel = Odjeli.ID OR Zaposleni.Odjel = 3 AND Zaposleni.Ime <= 'Dinko';");
//  printResults(rs);
//  delete rs;

  rs = db->ExecuteMQL(
  "SELECT Zaposleni.Ime, Zaposleni.Prezime, Odjeli.Naziv"
  " FROM Zaposleni, Odjeli"
  " WHERE Zaposleni.Odjel = Odjeli.ID;");
  printResults(rs);
  delete rs;

  delete db;

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
