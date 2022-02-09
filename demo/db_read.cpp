
#include <iostream>
#include <iomanip>

#include "MastersDB.h"

using namespace MastersDB;
using namespace std;


void printResults(MdbResultSet *rs);

const string QUERIES[6] = {
  "SELECT * FROM Employee;",
  "SELECT Firstname, Lastname FROM Employee;",
  "SELECT * FROM Department;",
  "SELECT * FROM Employee WHERE Firstname < 'I';",
  "SELECT Employee.Firstname, Employee.Lastname, Department.Name"
    " FROM Employee, Department"
    " WHERE Employee.DepartmentId = Department.Id;",
  "SELECT Employee.Firstname, Employee.Lastname, Department.Name"
    " FROM Employee, Department"
    " WHERE Employee.Firstname > 'D' AND Employee.Firstname < 'X'"
    " OR Employee.Lastname = 'Behram' AND Employee.DepartmentId = Department.Id;"
};

int main(int argc, char **argv)
{
  MdbDatabase *db;
  MdbResultSet *rs;

  db = MdbDatabase::OpenDatabase("test.mrdb");

  for (int q = 0; q < 6; q++) {
    cout << "  " << QUERIES[q] << endl;

    cout << "----------------------------------------------------------------------------------" << endl;
    rs = db->ExecuteMQL(QUERIES[q]);
    printResults(rs);
    cout << "----------------------------------------------------------------------------------" << endl;

    delete rs;
  }

  delete db;

  return 0;
}

void printResults(MdbResultSet *rs)
{
  uint8_t c;
  if (rs != NULL)
  {
    cout << "Retrieved " << rs->GetRecordCount() << " records!" << endl;
    cout << endl;
    for (c = 0; c < rs->GetColumnCount(); c++)
    {
      cout << left << setw(20) << rs->GetColumnName(c);
    }
    
    cout << endl << "----------------------------------------------------------------------------------";

    do
    {
      cout << endl;
      cout << left << setw(20);

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
        cout << left << setw(20);
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
