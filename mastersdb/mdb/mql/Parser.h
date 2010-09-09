

#if !defined(COCO_PARSER_H__)
#define COCO_PARSER_H__

extern "C" {
  #include "../mdb.h"
}

#include "../mvm/mdbVirtualMachine.h"
#include "MQLSelect.h"

#include <string>

using namespace MDB;
using namespace std;


#include "Scanner.h"



class Errors {
public:
	int count;			// number of errors detected

	Errors();
	void SynErr(int line, int col, int n);
	void Error(int line, int col, const wchar_t *s);
	void Warning(int line, int col, const wchar_t *s);
	void Warning(const wchar_t *s);
	void Exception(const wchar_t *s);

}; // Errors

class Parser {
private:
	enum {
		_EOF=0,
		_NUMBER=1,
		_IDENTIFIER=2,
		_COLUMN=3,
		_STRING=4,
	};
	int maxT;

	Token *dummyToken;
	int errDist;
	int minErrDist;

	void SynErr(int n);
	void Get();
	void Expect(int n);
	bool StartOf(int s);
	void ExpectWeak(int n, int follow);
	bool WeakSeparator(int n, int syFol, int repFol);

public:
	Scanner *scanner;
	Errors  *errors;

	Token *t;			// last recognized token
	Token *la;			// lookahead token

static const uint32 BUFFER_SIZE = 1024;
char buf[BUFFER_SIZE];
mdbVirtualMachine *VM;
MQLSelect *select;

uint16 dp;
uint8 tp;

string* TokenToString()
{
  wcstombs(buf, t->val, BUFFER_SIZE);
  return new string(buf);
}

uint32 getOffset(const void* src, const void* dest)
{
  return (uint32)((char*)dest - (char*)src);
}

void setVM (mdbVirtualMachine *vm)
{
  VM = vm;
}

void setSelect (MQLSelect *select)
{
  this->select = select;
}

/* ignores case */


	Parser();
	~Parser();
	void SemErr(const wchar_t* msg);

	void MQL();
	void MQLCreateStatement();
	void MQLInsertStatement();
	void MQLDescribeStatement();
	void MQLSelectStatement();
	void MQLAttributes();
	void MQLAttribute(bool first);
	void MQLDatatype(uint16* type_indexed, bool &has_length);
	void MQLValues();
	void MQLValue();
	void MQLColumns();
	void MQLTables();
	void MQLConditions();
	void MQLColumn(bool destination, mdbTableInfo* &ti);
	void MQLCondition(mdbOperation *op);
	void MQLConditionType(mdbOperation *op);

	void Parse(const unsigned char* buf, int len);

}; // end Parser



#endif // !defined(COCO_PARSER_H__)

