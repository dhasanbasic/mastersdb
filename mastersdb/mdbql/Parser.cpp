

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
			MQLCreate();
		} else if (la->kind == 16) {
			MQLInsert();
		} else SynErr(20);
		Expect(5);
		VM->AddInstruction(MastersDBVM::HALT, 0);    
}

void Parser::MQLCreate() {
		string *s;       
		char *name;      
		uint16* data;    
		uint16 num_cols; 
		uint16 ncp;      
		dp = 0;          
		tp = 0;          
		Expect(6);
		Expect(7);
		Expect(2);
		VM->AddInstruction(MastersDBVM::USETBL, tp); 
		s = TokenToString();                         
		name = new char[s->length() + 4];            
		*((uint32*)name) = s->length();              
		strcpy(name + 4, s->c_str());                
		delete s;                                    
		ncp = dp++;                                  
		VM->AddInstruction(MastersDBVM::PUSH, ncp);  
		VM->AddInstruction(MastersDBVM::ADDTBL, dp); 
		VM->Store(name, dp++);                       
		Expect(8);
		num_cols = 0;                                
		MQLAttributes(num_cols);
		data = new uint16;                           
		*data = num_cols;                            
		VM->Store((char*)data, ncp);                 
		Expect(9);
		VM->AddInstruction(MastersDBVM::CRTBL, tp);  
}

void Parser::MQLInsert() {
		string *s;       
		char *name;      
		dp = 0;          
		tp = 0;          
		Expect(16);
		Expect(17);
		Expect(2);
		VM->AddInstruction(MastersDBVM::USETBL, tp); 
		s = TokenToString();                         
		name = new char[s->length() + 4];            
		*((uint32*)name) = s->length();              
		strcpy(name + 4, s->c_str());                
		delete s;                                    
		VM->AddInstruction(MastersDBVM::LDTBL, dp);  
		VM->Store(name, dp++);                       
		Expect(18);
		Expect(8);
		MQLValues();
		Expect(9);
		VM->AddInstruction(MastersDBVM::INSTBL, tp); 
}

void Parser::MQLAttributes(uint16 &n) {
		mdbColumn *c;                                
		n = 0;                                       
		MQLAttribute(c);
		VM->AddInstruction(MastersDBVM::ADDCOL, dp); 
		VM->Store((char*)c, dp++);                   
		n++;                                         
		while (la->kind == 10) {
			VM->AddInstruction(MastersDBVM::ADDCOL, dp); 
			Get();
			MQLAttribute(c);
			VM->Store((char*)c, dp++);                   
			n++;                                         
		}
}

void Parser::MQLAttribute(mdbColumn* &c) {
		c = new mdbColumn;                 
		string *s;                         
		Expect(2);
		s = TokenToString();                         
		*((uint32*)c->name) = s->length();           
		strcpy(c->name + 4, s->c_str());             
		delete s;                                    
		MQLDatatype(c);
		c->length = 0;                               
		while (la->kind == 8) {
			Get();
			Expect(1);
			s = TokenToString();                         
			c->length = atoi(s->c_str());                
			delete s;                                    
			Expect(9);
		}
}

void Parser::MQLDatatype(mdbColumn *c) {
		if (la->kind == 11) {
			Get();
			c->type = 0; 
		} else if (la->kind == 12) {
			Get();
			c->type = 1; 
		} else if (la->kind == 13) {
			Get();
			c->type = 2; 
		} else if (la->kind == 14) {
			Get();
			c->type = 3; 
		} else if (la->kind == 15) {
			Get();
			c->type = 4; 
		} else SynErr(21);
}

void Parser::MQLValues() {
		MQLValue();
		while (la->kind == 10) {
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
			data = (char*)(new uint32);                  
			*data = atoi(s->c_str());                    
		} else if (la->kind == 3) {
			Get();
			s = TokenToString();                         
			data = new char[s->length() + 4];            
			*((uint32*)data) = s->length() - 2;          
			strncpy(data+4,s->c_str()+1,s->length()-2);  
		} else SynErr(22);
		VM->AddInstruction(MastersDBVM::ADDVAL, dp); 
		VM->Store(data, dp++);                       
		delete s;                                    
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
	maxT = 19;

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

	static bool set[1][21] = {
		{T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x}
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
			case 3: s = coco_string_create(L"STRING expected"); break;
			case 4: s = coco_string_create(L"OPERATOR expected"); break;
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
			case 19: s = coco_string_create(L"??? expected"); break;
			case 20: s = coco_string_create(L"invalid MQL"); break;
			case 21: s = coco_string_create(L"invalid MQLDatatype"); break;
			case 22: s = coco_string_create(L"invalid MQLValue"); break;

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



