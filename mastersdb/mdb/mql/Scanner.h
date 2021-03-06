

#if !defined(COCO_SCANNER_H__)
#define COCO_SCANNER_H__

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "Buffer.h"

using namespace MDB;

#define MIN_BUFFER_LENGTH 1024
#define MAX_BUFFER_LENGTH (64*MIN_BUFFER_LENGTH)
#define HEAP_BLOCK_SIZE (64*1024)
#define COCO_CPP_NAMESPACE_SEPARATOR L':'

// string handling, wide character
wchar_t* coco_string_create(const wchar_t *value);
wchar_t* coco_string_create(const wchar_t *value, int startIndex, int length);
wchar_t* coco_string_create_upper(const wchar_t* data);
wchar_t* coco_string_create_lower(const wchar_t* data);
wchar_t* coco_string_create_lower(const wchar_t* data, int startIndex, int dataLen);
wchar_t* coco_string_create_append(const wchar_t* data1, const wchar_t* data2);
wchar_t* coco_string_create_append(const wchar_t* data, const wchar_t value);
void  coco_string_delete(wchar_t* &data);
int   coco_string_length(const wchar_t* data);
bool  coco_string_endswith(const wchar_t* data, const wchar_t *value);
int   coco_string_indexof(const wchar_t* data, const wchar_t value);
int   coco_string_lastindexof(const wchar_t* data, const wchar_t value);
void  coco_string_merge(wchar_t* &data, const wchar_t* value);
bool  coco_string_equal(const wchar_t* data1, const wchar_t* data2);
int   coco_string_compareto(const wchar_t* data1, const wchar_t* data2);
int   coco_string_hash(const wchar_t* data);

// string handling, ascii character
wchar_t* coco_string_create(const char *value);
char* coco_string_create_char(const wchar_t *value);
void  coco_string_delete(char* &data);




class Token  
{
public:
	int kind;     // token kind
	int pos;      // token position in the source text (starting at 0)
	int col;      // token column (starting at 1)
	int line;     // token line (starting at 1)
	wchar_t* val; // token value
	Token *next;  // ML 2005-03-11 Peek tokens are kept in linked list

	Token();
	~Token();
};

//-----------------------------------------------------------------------------------
// StartStates  -- maps characters to start states of tokens
//-----------------------------------------------------------------------------------
class StartStates {
private:
	class Elem {
	public:
		int key, val;
		Elem *next;
		Elem(int key, int val) { this->key = key; this->val = val; next = NULL; }
	};

	Elem **tab;

public:
	StartStates() { tab = new Elem*[128]; memset(tab, 0, 128 * sizeof(Elem*)); }
	virtual ~StartStates() {
		for (int i = 0; i < 128; ++i) {
			Elem *e = tab[i];
			while (e != NULL) {
				Elem *next = e->next;
				delete e;
				e = next;
			}
		}
		delete [] tab;
	}

	void set(int key, int val) {
		Elem *e = new Elem(key, val);
		int k = ((unsigned int) key) % 128;
		e->next = tab[k]; tab[k] = e;
	}

	int state(int key) {
		Elem *e = tab[((unsigned int) key) % 128];
		while (e != NULL && e->key != key) e = e->next;
		return e == NULL ? 0 : e->val;
	}
};

//-------------------------------------------------------------------------------------------
// KeywordMap  -- maps strings to integers (identifiers to keyword kinds)
//-------------------------------------------------------------------------------------------
class KeywordMap {
private:
	class Elem {
	public:
		wchar_t *key;
		int val;
		Elem *next;
		Elem(const wchar_t *key, int val) { this->key = coco_string_create(key); this->val = val; next = NULL; }
		virtual ~Elem() { coco_string_delete(key); }
	};

	Elem **tab;

public:
	KeywordMap() { tab = new Elem*[128]; memset(tab, 0, 128 * sizeof(Elem*)); }
	virtual ~KeywordMap() {
		for (int i = 0; i < 128; ++i) {
			Elem *e = tab[i];
			while (e != NULL) {
				Elem *next = e->next;
				delete e;
				e = next;
			}
		}
		delete [] tab;
	}

	void set(const wchar_t *key, int val) {
		Elem *e = new Elem(key, val);
		int k = coco_string_hash(key) % 128;
		e->next = tab[k]; tab[k] = e;
	}

	int get(const wchar_t *key, int defaultVal) {
		Elem *e = tab[coco_string_hash(key) % 128];
		while (e != NULL && !coco_string_equal(e->key, key)) e = e->next;
		return e == NULL ? defaultVal : e->val;
	}
};

class Scanner {
private:
	void *firstHeap;
	void *heap;
	void *heapTop;
	void **heapEnd;

	unsigned char EOL;
	int eofSym;
	int noSym;
	int maxT;
	int charSetSize;
	StartStates start;
	KeywordMap keywords;

	Token *t;         // current token
	wchar_t *tval;    // text of current token
	int tvalLength;   // length of text of current token
	int tlen;         // length of current token

	Token *tokens;    // list of tokens already peeked (first token is a dummy)
	Token *pt;        // current peek token

	int ch;           // current input character
	wchar_t valCh;       // current input character (for token.val)

	int pos;          // byte position of current character
	int line;         // line number of current character
	int col;          // column number of current character
	int oldEols;      // EOLs that appeared in a comment;

	void CreateHeapBlock();
	Token* CreateToken();
	void AppendVal(Token *t);

	void Init();
	void NextCh();
	void AddCh();
	bool Comment0();

	Token* NextToken();

public:
	Buffer *buffer;   // scanner buffer
	
	Scanner(const unsigned char* buf, int len);
	~Scanner();
	Token* Scan();
	Token* Peek();
	void ResetPeek();

}; // end Scanner



#endif // !defined(COCO_SCANNER_H__)

