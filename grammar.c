#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "symbols.h"
#include "memoryImage.h"
#include "grammar.h"
#include "grammarHelper.h"

void buffer(Symbol *copy);
int parseInstruction(int instructionIndex, char **p_line, int lineNumber);

/* Prepares for instruction parsing. On error returns non-zero. */
int preInstruction(enum ParseMode mode, char *p_tmp, char **p_line, int lineNumber) {
	Symbol *s;
	char *instructionEnd = p_tmp;
	while (IsAlnum(&instructionEnd));
	*instructionEnd = '\0'; /* Replace space after instruction with \0 to mark end of instruction name */

	if (!((s = lookupSymbol(p_tmp)) && hasAttribute(s, INSTRUCTION_KEYWORD))) {
		printf("Error on line %d: Unknown instruction '%s'\n", lineNumber, p_tmp);
		return 1;
	}
	if (mode == PARSE_SYMBOLS) {
		imageExtend(CODE_IMAGE, WORD);
		**p_line = '\0'; /* Skip instruction part by ending line */
	}
	else if (parseInstruction(s->value, p_line, lineNumber))
		return 1;
		
	return 0;
}

/* Adds a symbol to the symbol table. On error returns non-zero. */
int addSymbols(enum ParseMode mode, char *p_tmp, int lineNumber, enum StateAction action) {
	if (mode == PARSE_SYMBOLS) {
		if (lookupSymbol(p_tmp)) {
			printf("Error on line %d: Symbol already defined\n", lineNumber);
			return 1;
		}
		if (strlen(p_tmp) > MAX_LABEL) {
			printf("Error on line %d: Maximum label length of %d characters is exceeded\n", lineNumber, MAX_LABEL);
			return 1;
		}
		if (!installSymbol(p_tmp, imageSize(action == AddDataSymbol ? DATA_IMAGE : CODE_IMAGE), action == AddDataSymbol ? DATA: CODE)) {
			printf("Error: Could not allocate memory for symbol\n");
			exit(1);
		}
	}
	return 0;
}

/* Writes the data to the memory image. On error returns non-zero. */
int writeData(enum ParseMode mode, char **p_line, int lineNumber, enum Type type) {
	char *p_tmp;
	long tmp;
	
	errno = 0;
	tmp = strtol(*p_line, &p_tmp, NUMBER_BASE);
	if (*p_line == p_tmp || errno == ERANGE) { /* Check that read a number */
		printf("Error on line %d: Missing number\n", lineNumber);
		return 1;
	}
	*p_line = p_tmp;
	if (mode == PARSE_SYMBOLS)
		imageExtend(DATA_IMAGE, type);
	else
		imageWriteBytes(DATA_IMAGE, tmp, type);
	return 0;
}

/* Set the symbol to 'entry'. On error returns non-zero. */
int doSetEntrySymbol(enum ParseMode mode, char *p_line, char *p_tmp, int lineNumber) {
	char tmp = *p_line;
	Symbol *s;
	if (mode == PARSE_ALL) {
		*p_line = '\0'; /* Mark end of symbol temporarily */
		if (!(s = lookupSymbol(p_tmp))) {
			printf("Error on line %d: Symbol used in entry not defined\n", lineNumber);
			return 1;
		}
		if (hasAttribute(s, INSTRUCTION_KEYWORD) || hasAttribute(s, DIRECTIVE_KEYWORD)) {
			printf("Error on line %d: Symbol '%s' is a defined keyword\n", lineNumber, s->name);
			return 1;
		}
		if (hasAttribute(s, EXTERNAL)) {
			printf("Error on line %d: Symbol '%s' can't be both external and entry\n", lineNumber, s->name);
			return 1;
		}
		if (!hasAttribute(s, ENTRY)) /* First time appearing as entry */
			buffer(s); /* Buffer to .ent file */
		*p_line = tmp;
		setAttribute(s, ENTRY);
	}
	return 0;
}

/* Set the symbol to 'external'. On error returns non-zero. */
int doSetExternSymbol(enum ParseMode mode, char *p_line, char *p_tmp, int lineNumber) {
	char tmp = *p_line;
	Symbol *s;
	*p_line = '\0'; /* Mark end of symbol temporarily */
	if ((s = lookupSymbol(p_tmp)) && !hasAttribute(s, EXTERNAL)) {
		printf("Error on line %d: Symbol '%s' is not allowed to also be extern\n", lineNumber, s->name);
		return 1;
	}
	if (!installSymbol(p_tmp, 0, EXTERNAL)) {
		printf("Error: Could not allocate memory for symbol\n");
		exit(1);
	}
	*p_line = tmp;
	return 0;
}

/* Returns the type's size when writing data */
enum Type getSizeType(int state) {
	switch (getStateAction(state)) {
		case WriteByte:
			return BYTE;
		case WriteHalf:
			return HALF;
		case WriteWord:
			return WORD;
		default:
			return 0;
	}
}

/* Parse a single line 'lineNumber' at p_line, using ParseMode 'mode'. Returns non-zero on error. */
int parseLine(int lineNumber, char *p_line, enum ParseMode mode) {
	char *p_tmp, *errorMsg;
	int state = 0;
	enum StateAction action;
	
	while (state != StateAccept) {
		/* Do predefined state action */
		action = getStateAction(state); 
		if (action == SavePosition)
			p_tmp = p_line;
		 if (action == NullPrevious)
			p_line[-1] = '\0';
		if (action == PrintWarn && mode == PARSE_SYMBOLS) /* Warnings will only be printed in first pass */
			printf("Warn on line %d: %s\n", lineNumber, getStateErrorMessage(state));
		if ((action == AddCodeSymbol || action == AddDataSymbol) && addSymbols(mode, p_tmp, lineNumber, action))
			return 1;
		if (action == InstructionParse && preInstruction(mode, p_tmp, &p_line, lineNumber))
			return 1;
		if (action == SetEntrySymbol && doSetEntrySymbol(mode, p_line, p_tmp, lineNumber))
			return 1;
		if (action == SetExternSymbol && doSetExternSymbol(mode, p_line, p_tmp, lineNumber))
			return 1;
		if (action == ReadComma && *p_line++ != ',') {
			printf("Error on line %d: Expected comma after parameter\n", lineNumber);
			return 1;
		}
		if ((action == WriteByte || action == WriteHalf || action == WriteWord) && writeData(mode, &p_line, lineNumber, getSizeType(state)))
			return 1;
		if (action == WriteChar || action == WriteTerminate) {
			if (mode == PARSE_SYMBOLS)
				imageExtend(DATA_IMAGE, BYTE);
			else
				imageWriteBytes(DATA_IMAGE, (action == WriteChar) ? p_line[-1] : '\0', BYTE);
		}

		/* Change state or print error message */
		errorMsg = getStateErrorMessage(state);
		if ((state = getNextState(state, &p_line)) == StateError) {
			printf("Error on line %d: %s\n", lineNumber, errorMsg);
			return 1;
		}
	}

	return 0;
}

/* Parses every line in a file. Returns non-zero on parsing error. */
int parse(FILE *stream, enum ParseMode mode) {
	char line[MAX_LINE + 1], c;
	int error = 0, lineNumber = 1;
	rewind(stream);

	while (line[MAX_LINE - 1] = '\0', fgets(line, MAX_LINE + 1, stream)) {
		/* Check if last character (before '\0') was written to, and is not a newline. 
			If so, read one more character, if possible that is not a newline. */
		if (line[MAX_LINE - 1] && line[MAX_LINE - 1] != '\n' && (c = fgetc(stream)) != EOF && c != '\n') {
			if (mode == PARSE_SYMBOLS) /* This message will only be printed in the first pass */
				printf("Error on line %d: Exceeds maximum length of %d characters\n", lineNumber, MAX_LINE);
			while ((c = fgetc(stream)) != EOF && c != '\n'); /* Read until end of line or file */
			error = 1;
		}
		else
			error |= parseLine(lineNumber, line, mode);
		lineNumber++;
	}

	return error;
}
