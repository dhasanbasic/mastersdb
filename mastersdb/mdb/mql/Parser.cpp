

#include <wchar.h>
#include "Parser.h"
#include "Scanner.h"




void Parser::SynErr(int n) {
	if (errDist >= minErrDist) errors->SynErr(la->line, la->col, n);
	errDist = 0;
}

void Parser::SemErr(const wchar_t* msg) {
	if (errDist >= minErrDist) errors->Error(t->line, t->col, msg);
	errDist = 0;
}

void Parser::Get() {
	for (;;) {
		t = la;
		la = scanner->Scan();
		if (la->kind <= maxT) { ++errDist; break; }

		if (dummyToken != t) {
			dummyToken->kind = t->kind;
			dummyToken->pos = t->pos;
			dummyToken->col = t->col;
			dummyToken->line = t->line;
			dummyToken->next = NULL;
			coco_string_delete(dummyToken->val);
			dummyToken->val = coco_string_create(t->val);
			t = dummyToken;
		}
		la = t;
	}
}

void Parser::Expect(int n) {
	if (la->kind==n) Get(); else { SynErr(n); }
}

void Parser::ExpectWeak(int n, int follow) {
	if (la->kind == n) Get();
	else {
		SynErr(n);
		while (!StartOf(follow)) Get();
	}
}

bool Parser::WeakSeparator(int n, int syFol, int repFol) {
	if (la->kind == n) {Get(); return true;}
	else if (StartOf(repFol)) {return false;}
	else {
		SynErr(n);
		while (!(StartOf(syFol) || StartOf(repFol) || StartOf(0))) {
			Get();
		}
		return StartOf(syFol);
	}
}

void Parser::MQL() {
		if (la->kind == 7) {
			MQLCreateStatement();
		} else if (la->kind == 17) {
			MQLInsertStatement();
		} else if (la->kind == 20 || la->kind == 21) {
			MQLDescribeStatement();
		} else if (la->kind == 22) {
			MQLSelectStatement();
		} else SynErr(26);
		Expect(6);
		VM->AddInstruction(mdbVirtualMachine::HALT,
		  mdbVirtualMachine::MVI_SUCCESS);
		
}

void Parser::MQLCreateStatement() {
		string *s;
		char *name;
		uint16 ncp;
		dp = 0;
		tp = 0;
		
		Expect(7);
		Expect(8);
		Expect(2);
		VM->AddInstruction(mdbVirtualMachine::SETTBL, tp);
		s = TokenToString();
		name = (char*)malloc(s->length() + 4);
		*((uint32*)name) = s->length();
		strncpy(name + 4, s->c_str(), s->length());
		delete s;
		ncp = dp;
		VM->StoreData(name, dp++);
		
		Expect(9);
		MQLAttributes();
		Expect(10);
		VM->AddInstruction(mdbVirtualMachine::CRTTBL, ncp); 
}

void Parser::MQLInsertStatement() {
		string *s;
		char *name;
		dp = 0;
		tp = 0;
		
		Expect(17);
		Expect(18);
		Expect(2);
		VM->AddInstruction(mdbVirtualMachine::SETTBL, tp);
		s = TokenToString();
		name = (char*)malloc(s->length() + 4);
		*((uint32*)name) = s->length();
		strncpy(name + 4, s->c_str(), s->length());
		delete s;
		VM->AddInstruction(mdbVirtualMachine::LDTBL, dp);
		VM->StoreData(name, dp++);
		
		Expect(19);
		Expect(9);
		MQLValues();
		Expect(10);
		VM->AddInstruction(mdbVirtualMachine::INSREC, tp); 
}

void Parser::MQLDescribeStatement() {
		string *s;
		char *name;
		dp = 0;
		tp = 0;
		
		if (la->kind == 20) {
			Get();
		} else if (la->kind == 21) {
			Get();
		} else SynErr(27);
		Expect(2);
		VM->AddInstruction(mdbVirtualMachine::SETTBL, tp);
		s = TokenToString();
		name = (char*)malloc(s->length() + 4);
		*((uint32*)name) = s->length();
		strncpy(name + 4, s->c_str(), s->length());
		delete s;
		VM->AddInstruction(mdbVirtualMachine::LDTBL, dp);
		VM->StoreData(name, dp++);
		VM->AddInstruction(mdbVirtualMachine::DSCTBL, tp);
		
}

void Parser::MQLSelectStatement() {
		dp = 0;
		tp = 0;
		select->Reset();
		
		Expect(22);
		MQLColumns();
		Expect(23);
		MQLTables();
		select->setDataPointer(dp);
		select->GenerateBytecode();
		
}

void Parser::MQLAttributes() {
		MQLAttribute(true);
		while (la->kind == 11) {
			Get();
			MQLAttribute(false);
		}
}

void Parser::MQLAttribute(bool first) {
		string *s;
		char *name;
		uint16* type_indexed;
		uint32* length = NULL;
		bool has_length;
		
		Expect(2);
		s = TokenToString();
		name = (char*)malloc(s->length() + 4);
		*((uint32*)name) = s->length();
		strncpy(name+4, s->c_str(), s->length());
		delete s;
		
		type_indexed = (uint16*)malloc(sizeof(uint16));
		*type_indexed = (first) ? 0xFF01 : 0xFF00;
		
		MQLDatatype(type_indexed, has_length);
		if (has_length) length = (uint32*)malloc(sizeof(uint32)); 
		while (la->kind == 9) {
			Get();
			Expect(1);
			s = TokenToString();
			*length = atoi(s->c_str());
			delete s;
			
			Expect(10);
		}
		if (has_length)
		{
		    // stores the column length and pushes its address
		    VM->AddInstruction(mdbVirtualMachine::PUSH, dp);
		    VM->StoreData((char*)length, dp++);
		}
		// stores the column type information and pushes its address
		VM->AddInstruction(mdbVirtualMachine::PUSH, dp);
		VM->StoreData((char*)type_indexed, dp++);
		// stores the column name
		VM->StoreData(name, dp);
		// now the column can be created
		VM->AddInstruction(mdbVirtualMachine::NEWCOL, dp++);
		
}

void Parser::MQLDatatype(uint16* type_indexed, bool &has_length) {
		if (la->kind == 12) {
			Get();
			(*type_indexed) &= 0x0001; has_length = false; 
		} else if (la->kind == 13) {
			Get();
			(*type_indexed) &= 0x0101; has_length = false; 
		} else if (la->kind == 14) {
			Get();
			(*type_indexed) &= 0x0201; has_length = false; 
		} else if (la->kind == 15) {
			Get();
			(*type_indexed) &= 0x0301; has_length = false; 
		} else if (la->kind == 16) {
			Get();
			(*type_indexed) &= 0x0401; has_length = true; 
		} else SynErr(28);
}

void Parser::MQLValues() {
		MQLValue();
		while (la->kind == 11) {
			Get();
			MQLValue();
		}
}

void Parser::MQLValue() {
		string *s;
		char *data;
		
		if (la->kind == 1) {
			Get();
			s = TokenToString();
			data = (char*)malloc(sizeof(uint32));
			*data = atoi(s->c_str());
			
		} else if (la->kind == 4) {
			Get();
			s = TokenToString();
			data = (char*)malloc(s->length() + 4);
			*((uint32*)data) = s->length() - 2;
			strncpy(data + 4, s->c_str() + 1, s->length() - 2);
			
		} else SynErr(29);
		VM->AddInstruction(mdbVirtualMachine::INSVAL, dp);
		VM->StoreData(data, dp++);
		delete s; 
		
}

void Parser::MQLColumns() {
		if (la->kind == 24) {
			string *table = NULL;
			string *column = NULL;
			
			Get();
			column = new string("*");
			if (select->MapColumn(column, table, dp))
			{
			   dp++;
			};
			delete column;
			
		} else if (la->kind == 2 || la->kind == 3) {
			MQLColumn();
			while (la->kind == 11) {
				Get();
				MQLColumn();
			}
		} else SynErr(30);
}

void Parser::MQLTables() {
		MQLTable();
		while (la->kind == 11) {
			Get();
			MQLTable();
		}
}

void Parser::MQLColumn() {
		string *table = NULL;
		string *column = NULL;
		string *tmp = NULL;
		uint32 dot;
		
		if (la->kind == 3) {
			Get();
			tmp = TokenToString();
			dot = tmp->find('.');
			table = new string(tmp->c_str(),dot);
			column = new string(tmp->c_str() + dot + 1);
			
		} else if (la->kind == 2) {
			Get();
			column = TokenToString(); 
		} else SynErr(31);
		if (select->MapColumn(column, table, dp))
		{
		   dp++;
		};
		delete table;
		delete column;
		delete tmp;
		
}

void Parser::MQLTable() {
		string *table = NULL; 
		Expect(2);
		table = TokenToString();
		select->MapTable(table, tp);
		tp++;
		delete table;
		
}



void Parser::Parse(const unsigned char* buf, int len) {
    delete scanner;
    delete errors;
	this->scanner = new Scanner(buf, len);
	errors = new Errors();
	minErrDist = 2;
	errDist = minErrDist;
	
	t = NULL;
	Get();
	MQL();

	Expect(0);
}

Parser::Parser() {
	maxT = 25;

	la = dummyToken = new Token();
	la->val = coco_string_create(L"Dummy Token");
	scanner = NULL;
	errors = NULL;
}

bool Parser::StartOf(int s) {
	const bool T = true;
	const bool x = false;

	static bool set[1][27] = {
		{T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x}
	};



	return set[s][la->kind];
}

Parser::~Parser() {
    delete scanner;
	delete errors;
	delete dummyToken;
}

Errors::Errors() {
	count = 0;
}

void Errors::SynErr(int line, int col, int n) {
	wchar_t* s;
	switch (n) {
			case 0: s = coco_string_create(L"EOF expected"); break;
			case 1: s = coco_string_create(L"NUMBER expected"); break;
			case 2: s = coco_string_create(L"IDENTIFIER expected"); break;
			case 3: s = coco_string_create(L"COLUMN expected"); break;
			case 4: s = coco_string_create(L"STRING expected"); break;
			case 5: s = coco_string_create(L"OPERATOR expected"); break;
			case 6: s = coco_string_create(L"\";\" expected"); break;
			case 7: s = coco_string_create(L"\"create\" expected"); break;
			case 8: s = coco_string_create(L"\"table\" expected"); break;
			case 9: s = coco_string_create(L"\"(\" expected"); break;
			case 10: s = coco_string_create(L"\")\" expected"); break;
			case 11: s = coco_string_create(L"\",\" expected"); break;
			case 12: s = coco_string_create(L"\"int-8\" expected"); break;
			case 13: s = coco_string_create(L"\"int-16\" expected"); break;
			case 14: s = coco_string_create(L"\"int-32\" expected"); break;
			case 15: s = coco_string_create(L"\"float\" expected"); break;
			case 16: s = coco_string_create(L"\"string\" expected"); break;
			case 17: s = coco_string_create(L"\"insert\" expected"); break;
			case 18: s = coco_string_create(L"\"into\" expected"); break;
			case 19: s = coco_string_create(L"\"values\" expected"); break;
			case 20: s = coco_string_create(L"\"desc\" expected"); break;
			case 21: s = coco_string_create(L"\"describe\" expected"); break;
			case 22: s = coco_string_create(L"\"select\" expected"); break;
			case 23: s = coco_string_create(L"\"from\" expected"); break;
			case 24: s = coco_string_create(L"\"*\" expected"); break;
			case 25: s = coco_string_create(L"??? expected"); break;
			case 26: s = coco_string_create(L"invalid MQL"); break;
			case 27: s = coco_string_create(L"invalid MQLDescribeStatement"); break;
			case 28: s = coco_string_create(L"invalid MQLDatatype"); break;
			case 29: s = coco_string_create(L"invalid MQLValue"); break;
			case 30: s = coco_string_create(L"invalid MQLColumns"); break;
			case 31: s = coco_string_create(L"invalid MQLColumn"); break;

		default:
		{
			wchar_t format[20];
			coco_swprintf(format, 20, L"error %d", n);
			s = coco_string_create(format);
		}
		break;
	}
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
	coco_string_delete(s);
	count++;
}

void Errors::Error(int line, int col, const wchar_t *s) {
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
	count++;
}

void Errors::Warning(int line, int col, const wchar_t *s) {
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
}

void Errors::Warning(const wchar_t *s) {
	wprintf(L"%ls\n", s);
}

void Errors::Exception(const wchar_t* s) {
	wprintf(L"%ls", s); 
	exit(1);
}



