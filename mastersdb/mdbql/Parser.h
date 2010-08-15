

#if !defined(COCO_PARSER_H__)
#define COCO_PARSER_H__

extern "C" {
  #include "../mastersdb.h"
}

#include "../mdbvm/MastersDBVM.h"

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
		_STRING=3,
		_OPERATOR=4,
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
MastersDBVM *VM;

uint16 dp;
uint16 tp;

string* TokenToString()
{
  wcstombs(buf, t->val, BUFFER_SIZE);
  return new string(buf);
}

uint32 getOffset(const void* src, const void* dest)
{
  return (uint32)((char*)dest - (char*)src);
}

void setVM (MastersDBVM *vm)
{
  VM = vm;
}

/* ignores case */


	Parser(Scanner *scanner);
	~Parser();
	void SemErr(const wchar_t* msg);

	void MQL();
	void MQLCreate();
	void MQLInsert();
	void MQLDescribe();
	void MQLAttributes(uint16 &n);
	void MQLAttribute(mdbColumnRecord* &c);
	void MQLDatatype(mdbColumnRecord *c);
	void MQLValues();
	void MQLValue();

	void Parse();

}; // end Parser



#endif // !defined(COCO_PARSER_H__)

