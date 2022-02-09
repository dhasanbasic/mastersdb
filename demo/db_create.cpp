
#include <iostream>
#include "MastersDB.h"

using namespace MastersDB;
using namespace std;

const string TABLES[2] = {
  "CREATE TABLE Department (Id INT-8, Name STRING(20));",
  "CREATE TABLE Employee (Firstname STRING(20), Lastname STRING(40), DepartmentId INT-8);"
};

const string DEPARTMENT_RECORDS[3] = {
  "INSERT INTO Department VALUES (1,'System Testing');",
  "INSERT INTO Department VALUES (2,'Development');",
  "INSERT INTO Department VALUES (3,'Poduct Management');"
};

const string EMPLOYEE_RECORDS[8] = {
  "INSERT INTO Employee VALUES ('Dinko','Hasanbašić', 2);",
  "INSERT INTO Employee VALUES ('Denis','Hasanbašić', 1);",
  "INSERT INTO Employee VALUES ('Alan','Verdict', 2);",
  "INSERT INTO Employee VALUES ('Peter','Müller', 3);",
  "INSERT INTO Employee VALUES ('Horst','König', 2);",
  "INSERT INTO Employee VALUES ('Swiss','Made', 1);",
  "INSERT INTO Employee VALUES ('Very','Famous', 3);",
  "INSERT INTO Employee VALUES ('Message','In-A-Bottle', 1);"
};


int main(int argc, char **argv)
{
  MdbDatabase *db;
  MdbResultSet *rs;

  db = MdbDatabase::CreateDatabase("test.mrdb");

  cout << "=============================================" << endl;
  cout << "              Creating tables                " << endl;
  cout << "=============================================" << endl;

  for (int t = 0; t < 2; t++) {
    cout << "  "  << TABLES[t] << endl;
    rs = db->ExecuteMQL(TABLES[t]);
  }

  cout << "=============================================" << endl;
  cout << "             Inserting records               " << endl;
  cout << "=============================================" << endl;

  cout << "DEPARTMENT:" << endl;

  for (int i = 0; i < 3; i++) {
    cout << "  "  << DEPARTMENT_RECORDS[i] << endl;
    rs = db->ExecuteMQL(DEPARTMENT_RECORDS[i]);
  }

  cout << endl << "EMPLOYEE:" << endl;

  for (int i = 0; i < 8; i++) {
    cout << "  "  << EMPLOYEE_RECORDS[i] << endl;
    rs = db->ExecuteMQL(EMPLOYEE_RECORDS[i]);
  }

  delete db;

  return 0;
}
