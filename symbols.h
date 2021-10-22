#ifndef SYMBOLS
#define SYMBOLS

enum Attribute {CODE=1, DATA=2, EXTERNAL=4, ENTRY=8, INSTRUCTION_KEYWORD=16, DIRECTIVE_KEYWORD=32};

typedef struct Symbol {
	char *name; /* defined name */
	int value;
	enum Attribute attribute;
	struct Symbol *next; /* next entry in chain */
} Symbol;

/* Test if symbol s has attribute a */
int hasAttribute(Symbol *s, enum Attribute a);

/* Set attribute a in symbol s */
void setAttribute(Symbol *s, enum Attribute a);

/* Look for symbol called name. If found returns pointer to symbol, otherwise NULL. */
Symbol *lookupSymbol(char *name);

/* Install a symbol, with a name, value, and attribute. Returns pointer to installed symbol or NULL on memory error. */
Symbol *installSymbol(char *name, int value, enum Attribute attribute);

/* Deletes all entries in hashtable that have one of 'attributes' */
void deleteTable(enum Attribute attributes);

#endif
