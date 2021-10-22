#include <stdlib.h>
#include <string.h>

#include "symbols.h"

#define HASHSIZE 211

static Symbol *hashtab[HASHSIZE]; /* pointers table */

/* hash: form hash value for string (Uses an SDBM Hash) */
unsigned hash(char *s) {
	unsigned hashval;
	for (hashval = 0; *s != '\0'; s++)
		hashval = *s + (hashval << 6) + (hashval << 16) - hashval;
	return hashval % HASHSIZE;
}

/* Look for symbol called name. If found returns pointer to symbol, otherwise NULL. */
Symbol *lookupSymbol(char *name) {
	Symbol *sp;
	for (sp = hashtab[hash(name)]; sp != NULL; sp = sp -> next)
		if (!strcmp(name, sp->name))
			return sp;
	return NULL;
}

char *strdup(char *s) {
	char *p;
	if ((p = (char *) malloc(strlen(s) + 1)) != NULL)
		strcpy(p, s);
	return p;
}

/* Install a symbol, with a name, value, and attribute. Returns pointer to installed symbol or NULL on memory error. */
Symbol *installSymbol(char *name, int value, enum Attribute attribute) {
	Symbol *sp;
	unsigned int hashval;

	if (!(sp = lookupSymbol(name))) { /* Not found */
		if (!((sp = (Symbol *) malloc(sizeof(Symbol))) && (sp->name = strdup(name)))) {
			free(sp); /* If symbol was allocated and strdup failed */
			return NULL;
		}
		hashval = hash(name);
		sp->next = hashtab[hashval];
		hashtab[hashval] = sp;
	}
	
	sp->value = value;
	sp->attribute = attribute;

	return sp;
}

/* Deletes all entries in hashtable that have one of 'attributes' */
void deleteTable(enum Attribute attributes) {
	int i;
	Symbol **walk, *tmp;

	/* Every iteration of outer loop, walk points to the next element in the hashtable - a pointer to Symbol */
	for (walk = hashtab, i = 0; i < HASHSIZE; walk = hashtab + ++i) {
		while (*walk != NULL) { /* While the pointer walk points to does not point to NULL */
			if ((*walk)->attribute & attributes) { /* Check if needs to be removed */
				*walk = (tmp = *walk)->next; /* Remove the Symbol that *walk points to, saving it in 'tmp' */
				free(tmp->name);
				free(tmp);
			}
			else /* If wasn't removed, move walk to point to next pointer to Symbol */
				walk = &(*walk)->next;
		}
	}
}

/* Symbol methods */

/* Test if symbol s has attribute a */
int hasAttribute(Symbol *s, enum Attribute a) {
	return s ? s->attribute & a : 0;
}

/* Set attribute a in symbol s */
void setAttribute(Symbol *s, enum Attribute a) {
	if (s)
		s->attribute |= a;
}
