#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include "grammarHelper.h"
#include "memoryImage.h"
#include "symbols.h"

void buffer(Symbol *copy);

#define NUM_REGISTERS 32
#define LEN_ADDRESS 25
#define OPCODE_OFFSET 26

int readRegister(int lineNumber, char **p_line, int *value) {
	long value_tmp;
	char *p_tmp;
	if (*(*p_line)++ != '$') {
		printf("Error on line %d: Expected register sign '$'\n", lineNumber);
		return 1;
	}
	if (!isdigit(**p_line)) {
		printf("Error on line %d: Register number should be directly prefixed with '$'\n", lineNumber);
		return 1;
	}
	errno = 0;
	value_tmp = strtol(*p_line, &p_tmp, NUMBER_BASE);
	if (*p_line == p_tmp || errno == ERANGE) {
		printf("Error on line %d: Register number invalid\n", lineNumber);
		return 1;
	}
	*p_line = p_tmp;
	if (!(0 <= value_tmp && value_tmp < NUM_REGISTERS)) {
		printf("Error on line %d: There are only %d registers (starting from 0)\n", lineNumber, NUM_REGISTERS);
		return 1;
	}
	*value = (int) value_tmp;
	return 0;
}

int readNumericConst(int lineNumber, char **p_line, int *value) {
	char *p_tmp;
	errno = 0;
	*value = (int) strtol(*p_line, &p_tmp, NUMBER_BASE);
	if (*p_line == p_tmp || errno == ERANGE) {
		printf("Error on line %d: Numeric constant is invalid\n", lineNumber);
		return 1;
	}
	*p_line = p_tmp;
	return 0;
}

int readLabel(int lineNumber, char **p_line, int *value, int internalOnly) {
	char *p_tmp = *p_line, tmp;
	Symbol *s;
	if (!isalpha(**p_line)) {
		printf("Error on line %d: Expected label starting with letter\n", lineNumber);
		return 1;
	}
	while (IsAlnum(p_line));
	tmp = **p_line;
	**p_line = '\0';
	if (!(s = lookupSymbol(p_tmp)) || hasAttribute(s, INSTRUCTION_KEYWORD) || hasAttribute(s, DIRECTIVE_KEYWORD)) {
		printf("Error on line %d: No such label '%s'\n", lineNumber, p_tmp);
		return 1;
	}
	if (!hasAttribute(s, EXTERNAL)) {
		*value = s->value + (hasAttribute(s, CODE) ? 0 : imageSize(CODE_IMAGE)) + 
					(internalOnly ? -imageCurrent(CODE_IMAGE) : CODE_START);
	} else if (internalOnly) {
		printf("Error on line %d: Label '%s' is external\n", lineNumber, p_tmp);
		return 1;
	} else {
		s->value = imageCurrent(CODE_IMAGE);
		buffer(s); /* Buffer to .ext file */
		*value = s->value = 0;
	}
	**p_line = tmp;
	return 0;
}

int readInternalLabel(int lineNumber, char **p_line, int *value) {
	return readLabel(lineNumber, p_line, value, 1);
}

int readAnyLabel(int lineNumber, char **p_line, int *value) {
	return readLabel(lineNumber, p_line, value, 0);
}

int readLabelOrRegister(int lineNumber, char **p_line, int *value) {
	if (**p_line == '$') {
		if (readRegister(lineNumber, p_line, value))
			return 1;
		*value |= 1 << LEN_ADDRESS;
	}
	else {
		if (readAnyLabel(lineNumber, p_line, value))
			return 1;
		*value &= ~(1 << LEN_ADDRESS);
	}
	return 0;
}


#define MAX_PARAMS 4
const static struct instruction {
	char *name;
	int opcode; /* Fits in 6 bits */
	int paramNumber;
	struct param {
		int startbit; /* The starting bit for encoding (least significant) */
		int length; /* Length of encoded param in bits */
		int fixed; /* The value of the param is set to this if 'eval' is NULL */
		int (*eval) (int lineNumber, char **p_line, int *value); /* Evaluate the value of this param given pointer to poisition in input. Return non-zero on error */
	} params[MAX_PARAMS];
} instructions[] = {
	/* R-type			rs							rt							rd							funct			*/
	{"add",		0,	4,	{{21, 5, 0, readRegister},	{16, 5, 0, readRegister},	{11, 5, 0, readRegister},	{6, 5, 1, NULL}}},
	{"sub",		0,	4,	{{21, 5, 0, readRegister},	{16, 5, 0, readRegister},	{11, 5, 0, readRegister},	{6, 5, 2, NULL}}},
	{"and",		0,	4,	{{21, 5, 0, readRegister},	{16, 5, 0, readRegister},	{11, 5, 0, readRegister},	{6, 5, 3, NULL}}},
	{"or",		0,	4,	{{21, 5, 0, readRegister},	{16, 5, 0, readRegister},	{11, 5, 0, readRegister},	{6, 5, 4, NULL}}},
	{"nor",		0,	4,	{{21, 5, 0, readRegister},	{16, 5, 0, readRegister},	{11, 5, 0, readRegister},	{6, 5, 5, NULL}}},
	/* Move     		rs							rd							funct			*/
	{"move",	1,	3,	{{21, 5, 0, readRegister},	{11, 5,  0, readRegister},	{6, 5, 1, NULL}}},
	{"mvhi",	1,	3,	{{21, 5, 0, readRegister},	{11, 5,  0, readRegister},	{6, 5, 2, NULL}}},
	{"mvlo",	1,	3,	{{21, 5, 0, readRegister},	{11, 5,  0, readRegister},	{6, 5, 3, NULL}}},
	/* I-type			rs							immed							rt			*/
	{"addi",	10,	3,	{{21, 5, 0, readRegister},	{0, 16, 0, readNumericConst},	{16, 5, 0, readRegister}}},
	{"subi",	11,	3,	{{21, 5, 0, readRegister},	{0, 16, 0, readNumericConst},	{16, 5, 0, readRegister}}},
	{"andi",	12,	3,	{{21, 5, 0, readRegister},	{0, 16, 0, readNumericConst},	{16, 5,  1, readRegister}}},
	{"ori",		13,	3,	{{21, 5, 0, readRegister},	{0, 16, 0, readNumericConst},	{16, 5, 0, readRegister}}},
	{"nori",	14,	3,	{{21, 5, 0, readRegister},	{0, 16, 0, readNumericConst},	{16, 5, 0, readRegister}}},
	/* Branching		rs							rt							immed		 */
	{"bne",		15, 3,	{{21, 5, 0, readRegister},	{16, 5, 0, readRegister},	{0, 16, 0, readInternalLabel}}},
	{"beq",		16, 3,	{{21, 5, 0, readRegister},	{16, 5, 0, readRegister},	{0, 16, 0, readInternalLabel}}},
	{"blt",		17, 3,	{{21, 5, 0, readRegister},	{16, 5, 0, readRegister},	{0, 16, 0, readInternalLabel}}},
	{"bgt",		18, 3,	{{21, 5, 0, readRegister},	{16, 5, 0, readRegister},	{0, 16, 0, readInternalLabel}}},
	/* Load/Save		rs							immed							rt			*/
	{"lb",		19,	3,	{{21, 5, 0, readRegister},	{0, 16, 0, readNumericConst},	{16, 5, 0, readRegister}}},
	{"sb",		20,	3,	{{21, 5, 0, readRegister},	{0, 16, 0, readNumericConst},	{16, 5, 0, readRegister}}},
	{"lw",		21,	3,	{{21, 5, 0, readRegister},	{0, 16, 0, readNumericConst},	{16, 5, 0, readRegister}}},
	{"sw",		22,	3,	{{21, 5, 0, readRegister},	{0, 16, 0, readNumericConst},	{16, 5, 0, readRegister}}},
	{"lh",		23,	3,	{{21, 5, 0, readRegister},	{0, 16, 0, readNumericConst},	{16, 5, 0, readRegister}}},
	{"sh",		24,	3,	{{21, 5, 0, readRegister},	{0, 16, 0, readNumericConst},	{16, 5, 0, readRegister}}},
	/* J-type 			address				*/
	{"jmp",		30,	2,	{{0, 26, 0, readLabelOrRegister}}},
	{"la",		31,	1,	{{0, 25, 0, readAnyLabel}}},
	{"call",	32,	1,	{{0, 25, 0, readAnyLabel}}},
	{"stop",	63,	0}
};

/* Adds keywords to symbol table, so that they can't be redfined */
void prepareInstructions(void) {
	int i;
	for (i = 0; i < sizeof instructions / sizeof (struct instruction); i++) {
		if (!installSymbol(instructions[i].name, i, INSTRUCTION_KEYWORD)) {
			printf("Error: Could not allocate required memory\n");
			exit(1);
		}
	}
	/* Directives */
	if (!(installSymbol("db", 0, DIRECTIVE_KEYWORD) && installSymbol("dh", 0, DIRECTIVE_KEYWORD) &&
			installSymbol("dw", 0, DIRECTIVE_KEYWORD) && installSymbol("asciz", 0, DIRECTIVE_KEYWORD) &&
			installSymbol("entry", 0, DIRECTIVE_KEYWORD) && installSymbol("extern", 0, DIRECTIVE_KEYWORD))) {
		printf("Error: Could not allocate required memory\n");
		exit(1);
	}
}

/* Builds an evaluated param into 'build' */
void buildParam(long *build, int startbit, int length, long value) {
	/* Add to build the value, constrained to length bits, shifted to startbit. */
	*build |= (value & ((1L << length) - 1)) << startbit;
}

/* Parse a single instruction with instructionIndex, params begin at *p_line */
int parseInstruction(int instructionIndex, char **p_line, int lineNumber) {
	int paramIndex, value;
	long build = 0;

	for (paramIndex = 0; paramIndex < instructions[instructionIndex].paramNumber; paramIndex++) {
		if (instructions[instructionIndex].params[paramIndex].eval != NULL) { /* Not a fixed value */
			/* comma state */
			if (paramIndex != 0) {
				if (*(*p_line)++ != ',') {
					printf("Error on line %d: Expected comma after parameter\n", lineNumber);
					return 1;
				}
				Spacing(p_line);
			}
			/* param state */
			if (instructions[instructionIndex].params[paramIndex].eval(lineNumber, p_line, &value))
				return 1;
			buildParam(&build, instructions[instructionIndex].params[paramIndex].startbit, 
						instructions[instructionIndex].params[paramIndex].length, value);
			Spacing(p_line);
		}
		else
			buildParam(&build, instructions[instructionIndex].params[paramIndex].startbit, 
						instructions[instructionIndex].params[paramIndex].length, instructions[instructionIndex].params[paramIndex].fixed);
	}
	buildParam(&build, OPCODE_OFFSET, (int) sizeof (char) * WORD - OPCODE_OFFSET, instructions[instructionIndex].opcode); /* Add opcode */
	imageWriteBytes(CODE_IMAGE, build, WORD);

	return 0;
}
