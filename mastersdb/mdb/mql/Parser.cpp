

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
		if (la->kind == 6) {
			MQLCreateStatement();
		} else if (la->kind == 16) {
			MQLInsertStatement();
		} else if (la->kind == 19 || la->kind == 20) {
			MQLDescribeStatement();
		} else if (la->kind == 21) {
			MQLSelectStatement();
		} else SynErr(34);
		Expect(5);
		VM->AddInstruction(mdbVirtualMachine::HALT,
		  mdbVirtualMachine::MVI_SUCCESS);
		
}

void Parser::MQLCreateStatement() {
		string *s;
		char *name;
		uint16 ncp;
		dp = 0;
		tp = 0;
		
		Expect(6);
		Expect(7);
		Expect(2);
		VM->AddInstruction(mdbVirtualMachine::SETTBL, tp);
		s = TokenToString();
		name = (char*)malloc(s->length() + 4);
		*((uint32*)name) = s->length();
		strncpy(name + 4, s->c_str(), s->length());
		delete s;
		ncp = dp;
		VM->StoreData(name, dp++);
		
		Expect(8);
		MQLAttributes();
		Expect(9);
		VM->AddInstruction(mdbVirtualMachine::CRTTBL, ncp); 
}

void Parser::MQLInsertStatement() {
		string *s;
		char *name;
		dp = 0;
		tp = 0;
		
		Expect(16);
		Expect(17);
		Expect(2);
		VM->AddInstruction(mdbVirtualMachine::SETTBL, tp);
		s = TokenToString();
		name = (char*)malloc(s->length() + 4);
		*((uint32*)name) = s->length();
		strncpy(name + 4, s->c_str(), s->length());
		delete s;
		VM->AddInstruction(mdbVirtualMachine::LDTBL, dp);
		VM->StoreData(name, dp++);
		
		Expect(18);
		Expect(8);
		MQLValues();
		Expect(9);
		VM->AddInstruction(mdbVirtualMachine::INSREC, tp); 
}

void Parser::MQLDescribeStatement() {
		string *s;
		char *name;
		dp = 0;
		tp = 0;
		
		if (la->kind == 19) {
			Get();
		} else if (la->kind == 20) {
			Get();
		} else SynErr(35);
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
		
		Expect(21);
		MQLColumns();
		Expect(22);
		MQLTables();
		if (la->kind == 23) {
			Get();
			MQLConditions();
		}
		select->setDataPointer(dp);
		select->GenerateBytecode();
		
}

void Parser::MQLAttributes() {
		MQLAttribute(true);
		while (la->kind == 10) {
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
		while (la->kind == 8) {
			Get();
			Expect(1);
			s = TokenToString();
			*length = atoi(s->c_str());
			delete s;
			
			Expect(9);
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
		if (la->kind == 11) {
			Get();
			(*type_indexed) &= 0x0001; has_length = false; 
		} else if (la->kind == 12) {
			Get();
			(*type_indexed) &= 0x0101; has_length = false; 
		} else if (la->kind == 13) {
			Get();
			(*type_indexed) &= 0x0201; has_length = false; 
		} else if (la->kind == 14) {
			Get();
			(*type_indexed) &= 0x0301; has_length = false; 
		} else if (la->kind == 15) {
			Get();
			(*type_indexed) &= 0x0401; has_length = true; 
		} else SynErr(36);
}

void Parser::MQLValues() {
		MQLValue();
		VM->AddInstruction(mdbVirtualMachine::INSVAL, dp++);
		
		while (la->kind == 10) {
			Get();
			MQLValue();
			VM->AddInstruction(mdbVirtualMachine::INSVAL, dp++);
			
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
			
		} else SynErr(37);
		VM->StoreData(data, dp);
		delete s; 
		
}

void Parser::MQLColumns() {
		mdbTableInfo *ti;
		string *table = NULL;
		string *column = NULL;
		
		if (la->kind == 24) {
			Get();
			column = new string("*");
			select->MapColumn(column, table, dp, ti, true);
			delete column;
			
		} else if (la->kind == 2 || la->kind == 3) {
			MQLColumn(true, ti);
			while (la->kind == 10) {
				Get();
				MQLColumn(true, ti);
			}
		} else SynErr(38);
}

void Parser::MQLTables() {
		string *table = NULL; 
		Expect(2);
		table = TokenToString();
		select->ResolveDefaultTable(table, dp);
		delete table;
		
		while (la->kind == 10) {
			Get();
			Expect(2);
		}
}

void Parser::MQLConditions() {
		mdbOperation *tmp;
		mdbOperation *op = new mdbOperation;
		memset(op, 0, sizeof(mdbOperation));
		
		MQLCondition(op);
		while (la->kind == 25 || la->kind == 26) {
			if (la->kind == 25) {
				Get();
				tmp = op;
				op = new mdbOperation;
				op->type = MDB_AND;  
				op->left_child = tmp;
				tmp = new mdbOperation;
				memset(tmp, 0, sizeof(mdbOperation));
				
			} else {
				Get();
				tmp = op;
				op = new mdbOperation;
				op->type = MDB_OR;  
				op->left_child = tmp;
				tmp = new mdbOperation;
				memset(tmp, 0, sizeof(mdbOperation));
				
			}
			MQLCondition(tmp);
			op->right_child = tmp; 
		}
		select->setOperation(op); 
}

void Parser::MQLColumn(bool destination, mdbTableInfo* &ti) {
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
		} else SynErr(39);
		select->MapColumn(column, table, dp, ti, destination);
		delete table;
		delete column;
		delete tmp;
		
}

void Parser::MQLCondition(mdbOperation *op) {
		mdbTableInfo *ti;
		uint8 tbl_left, tbl_right;
		uint16 col_left, col_right;
		uint32 param;
		uint32 *data;
		bool right_is_direct = false;
		
		MQLColumn(false, ti);
		tbl_left = ti->tp;
		col_left = ti->cdp;
		
		MQLConditionType(op);
		if (la->kind == 2 || la->kind == 3) {
			MQLColumn(false, ti);
			tbl_right = ti->tp;
			col_right = ti->cdp;
			
		} else if (la->kind == 1 || la->kind == 4) {
			MQLValue();
			tbl_right = 0;
			col_right = dp++;
			right_is_direct = true;
			
		} else SynErr(40);
		if (!right_is_direct && (tbl_left ^ tbl_right))
		{
		  select->addJoin(tbl_left);
		  select->addJoin(tbl_right);
		}
		
		// encodes the CMP instruction parameter and stores it)
		 param = (right_is_direct) ? 0x00000008 : 0;
		 param |= (uint8)op->type;
		 param = (param << 4) | tbl_left;
		 param = (param << 4) | tbl_right;
		 param = (param << 10) | col_left;
		 param = (param << 10) | col_right;
		 
		 data = (uint32*)malloc(sizeof(uint32));
		 *data = param;
		 
		 VM->StoreData((char*)data, dp);
		 op->param = param;
		 op->param_addr = dp++;  
		
}

void Parser::MQLConditionType(mdbOperation *op) {
		switch (la->kind) {
		case 27: {
			Get();
			op->type = MDB_LESS; 
			break;
		}
		case 28: {
			Get();
			op->type = MDB_GREATER; 
			break;
		}
		case 29: {
			Get();
			op->type = MDB_EQUAL; 
			break;
		}
		case 30: {
			Get();
			op->type = MDB_GREATER_OR_EQUAL; 
			break;
		}
		case 31: {
			Get();
			op->type = MDB_LESS_OR_EQUAL; 
			break;
		}
		case 32: {
			Get();
			op->type = MDB_NOT_EQUAL; 
			break;
		}
		default: SynErr(41); break;
		}
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
	maxT = 33;

	la = dummyToken = new Token();
	la->val = coco_string_create(L"Dummy Token");
	scanner = NULL;
	errors = NULL;
}

bool Parser::StartOf(int s) {
	const bool T = true;
	const bool x = false;

	static bool set[1][35] = {
		{T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x}
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
			case 5: s = coco_string_create(L"\";\" expected"); break;
			case 6: s = coco_string_create(L"\"create\" expected"); break;
			case 7: s = coco_string_create(L"\"table\" expected"); break;
			case 8: s = coco_string_create(L"\"(\" expected"); break;
			case 9: s = coco_string_create(L"\")\" expected"); break;
			case 10: s = coco_string_create(L"\",\" expected"); break;
			case 11: s = coco_string_create(L"\"int-8\" expected"); break;
			case 12: s = coco_string_create(L"\"int-16\" expected"); break;
			case 13: s = coco_string_create(L"\"int-32\" expected"); break;
			case 14: s = coco_string_create(L"\"float\" expected"); break;
			case 15: s = coco_string_create(L"\"string\" expected"); break;
			case 16: s = coco_string_create(L"\"insert\" expected"); break;
			case 17: s = coco_string_create(L"\"into\" expected"); break;
			case 18: s = coco_string_create(L"\"values\" expected"); break;
			case 19: s = coco_string_create(L"\"desc\" expected"); break;
			case 20: s = coco_string_create(L"\"describe\" expected"); break;
			case 21: s = coco_string_create(L"\"select\" expected"); break;
			case 22: s = coco_string_create(L"\"from\" expected"); break;
			case 23: s = coco_string_create(L"\"where\" expected"); break;
			case 24: s = coco_string_create(L"\"*\" expected"); break;
			case 25: s = coco_string_create(L"\"and\" expected"); break;
			case 26: s = coco_string_create(L"\"or\" expected"); break;
			case 27: s = coco_string_create(L"\"<\" expected"); break;
			case 28: s = coco_string_create(L"\">\" expected"); break;
			case 29: s = coco_string_create(L"\"=\" expected"); break;
			case 30: s = coco_string_create(L"\">=\" expected"); break;
			case 31: s = coco_string_create(L"\"<=\" expected"); break;
			case 32: s = coco_string_create(L"\"<>\" expected"); break;
			case 33: s = coco_string_create(L"??? expected"); break;
			case 34: s = coco_string_create(L"invalid MQL"); break;
			case 35: s = coco_string_create(L"invalid MQLDescribeStatement"); break;
			case 36: s = coco_string_create(L"invalid MQLDatatype"); break;
			case 37: s = coco_string_create(L"invalid MQLValue"); break;
			case 38: s = coco_string_create(L"invalid MQLColumns"); break;
			case 39: s = coco_string_create(L"invalid MQLColumn"); break;
			case 40: s = coco_string_create(L"invalid MQLCondition"); break;
			case 41: s = coco_string_create(L"invalid MQLConditionType"); break;

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



