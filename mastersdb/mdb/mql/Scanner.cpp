

#include <memory.h>
#include <string.h>
#include "Scanner.h"

// string handling, wide character

wchar_t* coco_string_create(const wchar_t* value) {
	wchar_t* data;
	int len = 0;
	if (value) { len = wcslen(value); }
	data = new wchar_t[len + 1];
	wcsncpy(data, value, len);
	data[len] = 0;
	return data;
}

wchar_t* coco_string_create(const wchar_t *value , int startIndex, int length) {
	int len = 0;
	wchar_t* data;

	if (value) { len = length; }
	data = new wchar_t[len + 1];
	wcsncpy(data, &(value[startIndex]), len);
	data[len] = 0;

	return data;
}

wchar_t* coco_string_create_upper(const wchar_t* data) {
	if (!data) { return NULL; }

	int dataLen = 0;
	if (data) { dataLen = wcslen(data); }

	wchar_t *newData = new wchar_t[dataLen + 1];

	for (int i = 0; i <= dataLen; i++) {
		if ((L'a' <= data[i]) && (data[i] <= L'z')) {
			newData[i] = data[i] + (L'A' - L'a');
		}
		else { newData[i] = data[i]; }
	}

	newData[dataLen] = L'\0';
	return newData;
}

wchar_t* coco_string_create_lower(const wchar_t* data) {
	if (!data) { return NULL; }
	int dataLen = wcslen(data);
	return coco_string_create_lower(data, 0, dataLen);
}

wchar_t* coco_string_create_lower(const wchar_t* data, int startIndex, int dataLen) {
	if (!data) { return NULL; }

	wchar_t* newData = new wchar_t[dataLen + 1];

	for (int i = 0; i <= dataLen; i++) {
		wchar_t ch = data[startIndex + i];
		if ((L'A' <= ch) && (ch <= L'Z')) {
			newData[i] = ch - (L'A' - L'a');
		}
		else { newData[i] = ch; }
	}
	newData[dataLen] = L'\0';
	return newData;
}

wchar_t* coco_string_create_append(const wchar_t* data1, const wchar_t* data2) {
	wchar_t* data;
	int data1Len = 0;
	int data2Len = 0;

	if (data1) { data1Len = wcslen(data1); }
	if (data2) {data2Len = wcslen(data2); }

	data = new wchar_t[data1Len + data2Len + 1];

	if (data1) { wcscpy(data, data1); }
	if (data2) { wcscpy(data + data1Len, data2); }

	data[data1Len + data2Len] = 0;

	return data;
}

wchar_t* coco_string_create_append(const wchar_t *target, const wchar_t appendix) {
	int targetLen = coco_string_length(target);
	wchar_t* data = new wchar_t[targetLen + 2];
	wcsncpy(data, target, targetLen);
	data[targetLen] = appendix;
	data[targetLen + 1] = 0;
	return data;
}

void coco_string_delete(wchar_t* &data) {
	delete [] data;
	data = NULL;
}

int coco_string_length(const wchar_t* data) {
	if (data) { return wcslen(data); }
	return 0;
}

bool coco_string_endswith(const wchar_t* data, const wchar_t *end) {
	int dataLen = wcslen(data);
	int endLen = wcslen(end);
	return (endLen <= dataLen) && (wcscmp(data + dataLen - endLen, end) == 0);
}

int coco_string_indexof(const wchar_t* data, const wchar_t value) {
	const wchar_t* chr = wcschr(data, value);

	if (chr) { return (chr-data); }
	return -1;
}

int coco_string_lastindexof(const wchar_t* data, const wchar_t value) {
	const wchar_t* chr = wcsrchr(data, value);

	if (chr) { return (chr-data); }
	return -1;
}

void coco_string_merge(wchar_t* &target, const wchar_t* appendix) {
	if (!appendix) { return; }
	wchar_t* data = coco_string_create_append(target, appendix);
	delete [] target;
	target = data;
}

bool coco_string_equal(const wchar_t* data1, const wchar_t* data2) {
	return wcscmp( data1, data2 ) == 0;
}

int coco_string_compareto(const wchar_t* data1, const wchar_t* data2) {
	return wcscmp(data1, data2);
}

int coco_string_hash(const wchar_t *data) {
	int h = 0;
	if (!data) { return 0; }
	while (*data != 0) {
		h = (h * 7) ^ *data;
		++data;
	}
	if (h < 0) { h = -h; }
	return h;
}

// string handling, ascii character

wchar_t* coco_string_create(const char* value) {
	int len = 0;
	if (value) { len = strlen(value); }
	wchar_t* data = new wchar_t[len + 1];
	for (int i = 0; i < len; ++i) { data[i] = (wchar_t) value[i]; }
	data[len] = 0;
	return data;
}

char* coco_string_create_char(const wchar_t *value) {
	int len = coco_string_length(value);
	char *res = new char[len + 1];
	for (int i = 0; i < len; ++i) { res[i] = (char) value[i]; }
	res[len] = 0;
	return res;
}

void coco_string_delete(char* &data) {
	delete [] data;
	data = NULL;
}




Token::Token() {
	kind = 0;
	pos  = 0;
	col  = 0;
	line = 0;
	val  = NULL;
	next = NULL;
}

Token::~Token() {
	coco_string_delete(val);
}

Scanner::Scanner(const unsigned char* buf, int len) {
    buffer = new Buffer(buf, len);
	Init();
}

Scanner::~Scanner() {
	char* cur = (char*) firstHeap;

	while(cur != NULL) {
		cur = *(char**) (cur + HEAP_BLOCK_SIZE);
		free(firstHeap);
		firstHeap = cur;
	}
	delete [] tval;
	delete buffer;
}

void Scanner::Init() {
	EOL    = '\n';
	eofSym = 0;
	maxT = 25;
	noSym = 25;
	int i;
	for (i = 48; i <= 57; ++i) start.set(i, 1);
	for (i = 97; i <= 104; ++i) start.set(i, 10);
	for (i = 106; i <= 122; ++i) start.set(i, 10);
	for (i = 196; i <= 196; ++i) start.set(i, 10);
	for (i = 214; i <= 214; ++i) start.set(i, 10);
	for (i = 220; i <= 220; ++i) start.set(i, 10);
	for (i = 228; i <= 228; ++i) start.set(i, 10);
	for (i = 246; i <= 246; ++i) start.set(i, 10);
	for (i = 252; i <= 252; ++i) start.set(i, 10);
	for (i = 262; i <= 263; ++i) start.set(i, 10);
	for (i = 268; i <= 269; ++i) start.set(i, 10);
	for (i = 272; i <= 273; ++i) start.set(i, 10);
	for (i = 352; i <= 353; ++i) start.set(i, 10);
	for (i = 381; i <= 382; ++i) start.set(i, 10);
	start.set(61, 9);
	start.set(34, 4);
	start.set(39, 6);
	start.set(60, 11);
	start.set(62, 12);
	start.set(59, 13);
	start.set(40, 14);
	start.set(41, 15);
	start.set(44, 16);
	start.set(105, 23);
	start.set(42, 22);
		start.set(Buffer::EoF, -1);
	keywords.set(L"create", 7);
	keywords.set(L"table", 8);
	keywords.set(L"float", 15);
	keywords.set(L"string", 16);
	keywords.set(L"insert", 17);
	keywords.set(L"into", 18);
	keywords.set(L"values", 19);
	keywords.set(L"desc", 20);
	keywords.set(L"describe", 21);
	keywords.set(L"select", 22);
	keywords.set(L"from", 23);


	tvalLength = 128;
	tval = new wchar_t[tvalLength]; // text of current token
	memset(tval, 0, tvalLength * sizeof(wchar_t));

	// HEAP_BLOCK_SIZE byte heap + pointer to next heap block
	heap = malloc(HEAP_BLOCK_SIZE + sizeof(void*));
	firstHeap = heap;
	heapEnd = (void**) (((char*) heap) + HEAP_BLOCK_SIZE);
	*heapEnd = 0;
	heapTop = heap;
	if (sizeof(Token) > HEAP_BLOCK_SIZE) {
		wprintf(L"--- Too small HEAP_BLOCK_SIZE\n");
		exit(1);
	}

	pos = -1; line = 1; col = 0;
	oldEols = 0;
	NextCh();
	if (ch == 0xEF) { // check optional byte order mark for UTF-8
		NextCh(); int ch1 = ch;
		NextCh(); int ch2 = ch;
		if (ch1 != 0xBB || ch2 != 0xBF) {
			wprintf(L"Illegal byte order mark at start of file");
			exit(1);
		}
		Buffer *oldBuf = buffer;
		buffer = new Buffer(buffer); col = 0;
		delete oldBuf; oldBuf = NULL;
		NextCh();
	}


	pt = tokens = CreateToken(); // first token is a dummy
}

void Scanner::NextCh() {
	if (oldEols > 0) { ch = EOL; oldEols--; }
	else {
		pos = buffer->GetPos();
		ch = buffer->Read(); col++;
		// replace isolated '\r' by '\n' in order to make
		// eol handling uniform across Windows, Unix and Mac
		if (ch == L'\r' && buffer->Peek() != L'\n') ch = EOL;
		if (ch == EOL) { line++; col = 0; }
	}
		valCh = ch;
		if ('A' <= ch && ch <= 'Z') ch = ch - 'A' + 'a'; // ch.ToLower()
}

void Scanner::AddCh() {
	if (tlen >= tvalLength) {
		tvalLength *= 2;
		wchar_t *newBuf = new wchar_t[tvalLength];
		memcpy(newBuf, tval, tlen*sizeof(wchar_t));
		delete [] tval;
		tval = newBuf;
	}
	if (ch != Buffer::EoF) {
		tval[tlen++] = valCh;
		NextCh();
	}
}


bool Scanner::Comment0() {
	int level = 1, pos0 = pos, line0 = line, col0 = col;
	NextCh();
	if (ch == L'*') {
		NextCh();
		for(;;) {
			if (ch == L'*') {
				NextCh();
				if (ch == L'/') {
					level--;
					if (level == 0) { oldEols = line - line0; NextCh(); return true; }
					NextCh();
				}
			} else if (ch == buffer->EoF) return false;
			else NextCh();
		}
	} else {
		buffer->SetPos(pos0); NextCh(); line = line0; col = col0;
	}
	return false;
}


void Scanner::CreateHeapBlock() {
	void* newHeap;
	char* cur = (char*) firstHeap;

	while(((char*) tokens < cur) || ((char*) tokens > (cur + HEAP_BLOCK_SIZE))) {
		cur = *((char**) (cur + HEAP_BLOCK_SIZE));
		free(firstHeap);
		firstHeap = cur;
	}

	// HEAP_BLOCK_SIZE byte heap + pointer to next heap block
	newHeap = malloc(HEAP_BLOCK_SIZE + sizeof(void*));
	*heapEnd = newHeap;
	heapEnd = (void**) (((char*) newHeap) + HEAP_BLOCK_SIZE);
	*heapEnd = 0;
	heap = newHeap;
	heapTop = heap;
}

Token* Scanner::CreateToken() {
	Token *t;
	if (((char*) heapTop + (int) sizeof(Token)) >= (char*) heapEnd) {
		CreateHeapBlock();
	}
	t = (Token*) heapTop;
	heapTop = (void*) ((char*) heapTop + sizeof(Token));
	t->val = NULL;
	t->next = NULL;
	return t;
}

void Scanner::AppendVal(Token *t) {
	int reqMem = (tlen + 1) * sizeof(wchar_t);
	if (((char*) heapTop + reqMem) >= (char*) heapEnd) {
		if (reqMem > HEAP_BLOCK_SIZE) {
			wprintf(L"--- Too long token value\n");
			exit(1);
		}
		CreateHeapBlock();
	}
	t->val = (wchar_t*) heapTop;
	heapTop = (void*) ((char*) heapTop + reqMem);

	wcsncpy(t->val, tval, tlen);
	t->val[tlen] = L'\0';
}

Token* Scanner::NextToken() {
	while (ch == ' ' ||
			(ch >= 9 && ch <= 13) || ch == L' '
	) NextCh();
	if ((ch == L'/' && Comment0())) return NextToken();
	t = CreateToken();
	t->pos = pos; t->col = col; t->line = line;
	int state = start.state(ch);
	tlen = 0; AddCh();

	switch (state) {
		case -1: { t->kind = eofSym; break; } // NextCh already done
		case 0: { t->kind = noSym; break; }   // NextCh already done
		case 1:
			case_1:
			if ((ch >= L'0' && ch <= L'9')) {AddCh(); goto case_1;}
			else {t->kind = 1; break;}
		case 2:
			case_2:
			if ((ch >= L'a' && ch <= L'z') || ch == 196 || ch == 214 || ch == 220 || ch == 228 || ch == 246 || ch == 252 || (ch >= 262 && ch <= 263) || (ch >= 268 && ch <= 269) || (ch >= 272 && ch <= 273) || (ch >= 352 && ch <= 353) || (ch >= 381 && ch <= 382)) {AddCh(); goto case_3;}
			else {t->kind = noSym; break;}
		case 3:
			case_3:
			if ((ch >= L'a' && ch <= L'z') || ch == 196 || ch == 214 || ch == 220 || ch == 228 || ch == 246 || ch == 252 || (ch >= 262 && ch <= 263) || (ch >= 268 && ch <= 269) || (ch >= 272 && ch <= 273) || (ch >= 352 && ch <= 353) || (ch >= 381 && ch <= 382)) {AddCh(); goto case_3;}
			else {t->kind = 3; break;}
		case 4:
			case_4:
			if (ch <= L'!' || (ch >= L'#' && ch <= L'[') || (ch >= L']' && ch <= 65535)) {AddCh(); goto case_4;}
			else if (ch == L'"') {AddCh(); goto case_8;}
			else if (ch == 92) {AddCh(); goto case_5;}
			else {t->kind = noSym; break;}
		case 5:
			case_5:
			if (ch == 39) {AddCh(); goto case_4;}
			else {t->kind = noSym; break;}
		case 6:
			case_6:
			if (ch <= L'&' || (ch >= L'(' && ch <= L'[') || (ch >= L']' && ch <= 65535)) {AddCh(); goto case_6;}
			else if (ch == 39) {AddCh(); goto case_8;}
			else if (ch == 92) {AddCh(); goto case_7;}
			else {t->kind = noSym; break;}
		case 7:
			case_7:
			if (ch == L'"') {AddCh(); goto case_6;}
			else {t->kind = noSym; break;}
		case 8:
			case_8:
			{t->kind = 4; break;}
		case 9:
			case_9:
			{t->kind = 5; break;}
		case 10:
			case_10:
			if ((ch >= L'a' && ch <= L'z') || ch == 196 || ch == 214 || ch == 220 || ch == 228 || ch == 246 || ch == 252 || (ch >= 262 && ch <= 263) || (ch >= 268 && ch <= 269) || (ch >= 272 && ch <= 273) || (ch >= 352 && ch <= 353) || (ch >= 381 && ch <= 382)) {AddCh(); goto case_10;}
			else if (ch == L'.') {AddCh(); goto case_2;}
			else {t->kind = 2; wchar_t *literal = coco_string_create_lower(tval, 0, tlen); t->kind = keywords.get(literal, t->kind); coco_string_delete(literal); break;}
		case 11:
			if ((ch >= L'=' && ch <= L'>')) {AddCh(); goto case_9;}
			else {t->kind = 5; break;}
		case 12:
			if (ch == L'=') {AddCh(); goto case_9;}
			else {t->kind = 5; break;}
		case 13:
			{t->kind = 6; break;}
		case 14:
			{t->kind = 9; break;}
		case 15:
			{t->kind = 10; break;}
		case 16:
			{t->kind = 11; break;}
		case 17:
			case_17:
			{t->kind = 12; break;}
		case 18:
			case_18:
			if (ch == L'6') {AddCh(); goto case_19;}
			else {t->kind = noSym; break;}
		case 19:
			case_19:
			{t->kind = 13; break;}
		case 20:
			case_20:
			if (ch == L'2') {AddCh(); goto case_21;}
			else {t->kind = noSym; break;}
		case 21:
			case_21:
			{t->kind = 14; break;}
		case 22:
			{t->kind = 24; break;}
		case 23:
			if ((ch >= L'a' && ch <= L'm') || (ch >= L'o' && ch <= L'z') || ch == 196 || ch == 214 || ch == 220 || ch == 228 || ch == 246 || ch == 252 || (ch >= 262 && ch <= 263) || (ch >= 268 && ch <= 269) || (ch >= 272 && ch <= 273) || (ch >= 352 && ch <= 353) || (ch >= 381 && ch <= 382)) {AddCh(); goto case_10;}
			else if (ch == L'.') {AddCh(); goto case_2;}
			else if (ch == L'n') {AddCh(); goto case_24;}
			else {t->kind = 2; wchar_t *literal = coco_string_create_lower(tval, 0, tlen); t->kind = keywords.get(literal, t->kind); coco_string_delete(literal); break;}
		case 24:
			case_24:
			if ((ch >= L'a' && ch <= L's') || (ch >= L'u' && ch <= L'z') || ch == 196 || ch == 214 || ch == 220 || ch == 228 || ch == 246 || ch == 252 || (ch >= 262 && ch <= 263) || (ch >= 268 && ch <= 269) || (ch >= 272 && ch <= 273) || (ch >= 352 && ch <= 353) || (ch >= 381 && ch <= 382)) {AddCh(); goto case_10;}
			else if (ch == L'.') {AddCh(); goto case_2;}
			else if (ch == L't') {AddCh(); goto case_25;}
			else {t->kind = 2; wchar_t *literal = coco_string_create_lower(tval, 0, tlen); t->kind = keywords.get(literal, t->kind); coco_string_delete(literal); break;}
		case 25:
			case_25:
			if ((ch >= L'a' && ch <= L'z') || ch == 196 || ch == 214 || ch == 220 || ch == 228 || ch == 246 || ch == 252 || (ch >= 262 && ch <= 263) || (ch >= 268 && ch <= 269) || (ch >= 272 && ch <= 273) || (ch >= 352 && ch <= 353) || (ch >= 381 && ch <= 382)) {AddCh(); goto case_10;}
			else if (ch == L'.') {AddCh(); goto case_2;}
			else if (ch == L'-') {AddCh(); goto case_26;}
			else {t->kind = 2; wchar_t *literal = coco_string_create_lower(tval, 0, tlen); t->kind = keywords.get(literal, t->kind); coco_string_delete(literal); break;}
		case 26:
			case_26:
			if (ch == L'8') {AddCh(); goto case_17;}
			else if (ch == L'1') {AddCh(); goto case_18;}
			else if (ch == L'3') {AddCh(); goto case_20;}
			else {t->kind = noSym; break;}

	}
	AppendVal(t);
	return t;
}

// get the next token (possibly a token already seen during peeking)
Token* Scanner::Scan() {
	if (tokens->next == NULL) {
		return pt = tokens = NextToken();
	} else {
		pt = tokens = tokens->next;
		return tokens;
	}
}

// peek for the next token, ignore pragmas
Token* Scanner::Peek() {
	do {
		if (pt->next == NULL) {
			pt->next = NextToken();
		}
		pt = pt->next;
	} while (pt->kind > maxT); // skip pragmas

	return pt;
}

// make sure that peeking starts at the current scan position
void Scanner::ResetPeek() {
	pt = tokens;
}



