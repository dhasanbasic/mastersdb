

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
		VM->AddInstruction(MastersDBVM::HALT,        
		MastersDBVM::MVI_SUCCESS);                 
}

void Parser::MQLCreateStatement() {
		string *s;       
		char *name;      
		uint16* data;    
		uint16 num_cols; 
		uint16 ncp;      
		dp = 0;          
		tp = 0;          
		Expect(7);
		Expect(8);
		Expect(2);
		VM->AddInstruction(MastersDBVM::SETTBL, tp); 
		s = TokenToString();                         
		name = (char*)malloc(s->length() + 4);       
		*((uint32*)name) = s->length();              
		strncpy(name + 4, s->c_str(), s->length());  
		delete s;                                    
		ncp = dp++;                                  
		VM->AddInstruction(MastersDBVM::PUSHM, ncp); 
		VM->AddInstruction(MastersDBVM::NEWTBL, dp); 
		VM->StoreData(name, dp++);                   
		Expect(9);
		num_cols = 0;                                
		MQLAttributes(num_cols);
		data = (uint16*)malloc(sizeof(uint16));      
		*data = num_cols;                            
		VM->StoreData((char*)data, ncp);             
		Expect(10);
		VM->AddInstruction(MastersDBVM::CRTTBL, tp); 
}

void Parser::MQLInsertStatement() {
		string *s;       
		char *name;      
		dp = 0;          
		tp = 0;          
		Expect(17);
		Expect(18);
		Expect(2);
		VM->AddInstruction(MastersDBVM::SETTBL, tp); 
		s = TokenToString();                         
		name = (char*)malloc(s->length() + 4);       
		*((uint32*)name) = s->length();              
		strncpy(name + 4, s->c_str(), s->length());  
		delete s;                                    
		VM->AddInstruction(MastersDBVM::LDTBL, dp);  
		VM->StoreData(name, dp++);                   
		Expect(19);
		Expect(9);
		MQLValues();
		Expect(10);
		VM->AddInstruction(MastersDBVM::INSREC, tp); 
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
		VM->AddInstruction(MastersDBVM::SETTBL, tp); 
		s = TokenToString();                         
		name = (char*)malloc(s->length() + 4);       
		*((uint32*)name) = s->length();              
		strncpy(name + 4, s->c_str(), s->length());  
		delete s;                                    
		VM->AddInstruction(MastersDBVM::LDTBL, dp);  
		VM->StoreData(name, dp++);                   
		VM->AddInstruction(MastersDBVM::DSCTBL, tp); 
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

void Parser::MQLAttributes(uint16 &n) {
		mdbColumnRecord *c;                          
		n = 0;                                       
		MQLAttribute(c);
		VM->AddInstruction(MastersDBVM::NEWCOL, dp); 
		VM->StoreData((char*)c, dp++);               
		n++;                                         
		while (la->kind == 11) {
			VM->AddInstruction(MastersDBVM::NEWCOL, dp); 
			Get();
			MQLAttribute(c);
			VM->StoreData((char*)c, dp++);               
			n++;                                         
		}
}

void Parser::MQLAttribute(mdbColumnRecord* &c) {
		string *s;                         
		c = (mdbColumnRecord*)                       
		malloc(sizeof(mdbColumnRecord));      
		memset(c, 0, sizeof(mdbColumnRecord));       
		Expect(2);
		s = TokenToString();                         
		*((uint32*)c->name) = s->length();           
		strncpy(c->name+4, s->c_str(), s->length()); 
		delete s;                                    
		MQLDatatype(c);
		c->length = 0;                               
		while (la->kind == 9) {
			Get();
			Expect(1);
			s = TokenToString();                         
			c->length = atoi(s->c_str());                
			delete s;                                    
			Expect(10);
		}
}

void Parser::MQLDatatype(mdbColumnRecord *c) {
		if (la->kind == 12) {
			Get();
			c->type = 0; 
		} else if (la->kind == 13) {
			Get();
			c->type = 1; 
		} else if (la->kind == 14) {
			Get();
			c->type = 2; 
		} else if (la->kind == 15) {
			Get();
			c->type = 3; 
		} else if (la->kind == 16) {
			Get();
			c->type = 4; 
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
			strncpy(data+4,s->c_str()+1,s->length()-2);  
		} else SynErr(29);
		VM->AddInstruction(MastersDBVM::INSVAL, dp); 
		VM->StoreData(data, dp++);                   
		delete s;                                    
}

void Parser::MQLColumns() {
		if (la->kind == 24) {
			Get();
			select->UseAllColumns();                     
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
			column = new string(tmp->c_str()+dot+1);     
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



void Parser::Parse() {
	t = NULL;
	la = dummyToken = new Token();
	la->val = coco_string_create(L"Dummy Token");
	Get();
	MQL();

	Expect(0);
}

Parser::Parser(Scanner *scanner) {
	maxT = 25;

	dummyToken = NULL;
	t = la = NULL;
	minErrDist = 2;
	errDist = minErrDist;
	this->scanner = scanner;
	errors = new Errors();
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



