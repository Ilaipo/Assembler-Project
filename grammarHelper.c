#include <ctype.h>
#include <stdlib.h>

#include "grammarHelper.h"

/* Returns true if 'cs' starts with 'ct' */
int startswith(const char *cs, const char *ct) {
	while (*cs && *ct && *cs == *ct) cs++, ct++;
	return *ct == '\0';
}

/* The following functions are used as conditions in the state table. They all have one char ** paramater and return True or False. 
* If the condition is met, the functions change the position of the pointer used for reading the line (p_line). */

int Spacing(char **p_line) {
	if (!(isspace(**p_line) && **p_line != '\n')) return 0;
	while (isspace(**p_line) && **p_line != '\n') (*p_line)++;
	return 1;
}
int End(char **p_line) {
	char *p_tmp = *p_line;
	while (isspace(*p_tmp)) p_tmp++;
	if (*p_tmp != '\0') return 0;
	*p_line = p_tmp;
	return 1;
}
int CommentStart(char **p_line) {
	if (**p_line != ';') return 0;
	(*p_line)++;
	return 1;
}
int DirectiveStart(char **p_line) {
	if (**p_line != '.') return 0;
	(*p_line)++;
	return 1;
}
int Default(char **p_line) {
	return 1;
}
int IsEntry(char **p_line) {
	if (!startswith(*p_line, "entry")) return 0;
	(*p_line) += 5; /* Length of 'entry' */
	return 1;
}
int IsExtern(char **p_line) {
	if (!startswith(*p_line, "extern")) return 0;
	(*p_line) += 6; /* Length of 'extern' */
	return 1;
}
int IsBytes(char **p_line) {
	if (!startswith(*p_line, "db")) return 0;
	(*p_line) += 2; /* Length of 'db' */
	return 1;
}
int IsHalves(char **p_line) {
	if (!startswith(*p_line, "dh")) return 0;
	(*p_line) += 2; /* Length of 'dh' */
	return 1;
}
int IsWords(char **p_line) {
	if (!startswith(*p_line, "dw")) return 0;
	(*p_line) += 2; /* Length of 'dw' */
	return 1;
}
int IsAscii(char **p_line) {
	if (!startswith(*p_line, "asciz")) return 0;
	(*p_line) += 5; /* Length of 'asciz' */
	return 1;
}
int IsAlpha(char **p_line) {
	if (!isalpha(**p_line)) return 0;
	(*p_line)++;
	return 1;
}
int IsAlnum(char **p_line) {
	if (!isalnum(**p_line)) return 0;
	(*p_line)++;
	return 1;
}
int IsPrint(char **p_line) {
	if (!isprint(**p_line)) return 0;
	(*p_line)++;
	return 1;
}
int LabelMarker(char **p_line) {
	if (**p_line != ':') return 0;
	(*p_line)++;
	return 1;
}
int Quotation(char **p_line) {
	if (**p_line != '\"') return 0;
	(*p_line)++;
	return 1;
}

/* The state table that defines a state machine to parse the grammar */

#define MAX_CONDITIONS 5
const static struct State {
	enum StateAction stateAction; /* A predefined action that will be executed on changing to the state */
	int (*conditions[MAX_CONDITIONS])(char **p_line); /* An array of conditions for changing states */
	int nextStates[MAX_CONDITIONS]; /* An array of next state indices matching the conditions */
	char *errorMessage; /* An error message to print, if no condition is matched */
} States[] = {
/* 0 - Statement */						{ Nothing, {Spacing, End, CommentStart, DirectiveStart, Default}, {0, StateAccept, StateAccept, 1, 2}, "" },
/* 1 - Directive */						{ Nothing, {IsEntry, IsExtern, Default}, {11, 12, 17}, "" },
/* 2 - LabelOrInstructionStart */		{ SavePosition, {IsAlpha}, {3}, "Labels and instructions must start with a letter"},
/* 3 - LabelOrInstructionTail */		{ Nothing, {IsAlnum, LabelMarker, Default}, {3, 4, 15}, "" },
/* 4 - LabelEnd */						{ NullPrevious, {Spacing}, {5}, "Expected space after label's ':'" },
/* 5 - LabeledInstructionOrDirective */ { Nothing, {Spacing, DirectiveStart, Default}, {5, 6, 10}, "" },
/* 6 - LabeledDirective */				{ Nothing, {IsEntry, IsExtern, Default}, {7, 8, 9}, "" },
/* 7 - LabeledEntry */					{ PrintWarn, {Default}, {11}, "Label on entry directive is meaningless and is ignored" },
/* 8 - LabeledExtern */					{ PrintWarn, {Default}, {12}, "Label on extern directive is meaningless and is ignored" },
/* 9 - LabeledData */					{ AddDataSymbol, {Default}, {17}, "" },
/* 10 - LabeledInstruction */			{ AddCodeSymbol, {Default}, {13}, "" },
/* 11 - Entry */						{ Nothing, {Spacing}, {18}, "Expected space after 'entry'" },
/* 12 - Extern */						{ Nothing, {Spacing}, {21}, "Expected space after 'extern'" },
/* 13 - InstructionStart */				{ SavePosition, {IsAlpha}, {14}, "Expected instruction" },
/* 14 - InstructionTail */				{ Nothing, {IsAlnum, Default}, {14, 15}, "" },
/* 15 - InstructionEnd */				{ Nothing, {Spacing, End}, {16, 16}, "Invalid character in label or instruction" },
/* 16 - Instruction */					{ InstructionParse, {End}, {StateAccept}, "Extraneous text after parameters" },
/* 17 - Data */							{ Nothing, {IsBytes, IsHalves, IsWords, IsAscii}, {25, 26, 27, 28}, "Unrecognized directive" },
/* 18 - EntryParameterStart */			{ SavePosition, {IsAlpha}, {19}, "Label must start with a letter" },
/* 19 - EntryParameterTail */			{ Nothing, {IsAlnum, Default}, {19, 20}, "" },
/* 20 - EntryParameterEnd */			{ SetEntrySymbol, {Default}, {24}, "" },
/* 21 - ExternParameterStart */			{ SavePosition, {IsAlpha}, {22}, "Label did not start with letter" },
/* 22 - ExternParameterTail */			{ Nothing, {IsAlnum, Default}, {22, 23}, "" },
/* 23 - ExternParameterEnd */			{ SetExternSymbol, {Default}, {24}, "" },
/* 24 - TrailingSpace */				{ Nothing, {Spacing, End}, {24, StateAccept}, "Extraneous text after parameter" },
/* 25 - Bytes */						{ Nothing, {Spacing}, {29}, "Expected space after directive" },
/* 26 - Halves */						{ Nothing, {Spacing}, {31}, "Expected space after directive" },
/* 27 - Words */						{ Nothing, {Spacing}, {33}, "Expected space after directive" },
/* 28 - Ascii */						{ Nothing, {Spacing}, {35}, "Expected space after directive" },
/* 29 - Byte */							{ WriteByte, {Spacing, End, Default}, {30, StateAccept, 30}, "" },
/* 30 - ByteSep */						{ ReadComma, {Spacing, Default}, {29, 29}, "" },
/* 31 - Half */							{ WriteHalf, {Spacing, End, Default}, {32, StateAccept, 32}, "" },
/* 32 - HalfSep */						{ ReadComma, {Spacing, Default}, {31, 31}, "" },
/* 33 - Word */							{ WriteWord, {Spacing, End, Default}, {34, StateAccept, 34}, "" },
/* 34 - WordSep */						{ ReadComma, {Spacing, Default}, {33, 33}, "" },
/* 35 - StringStart */					{ Nothing, {Quotation}, {36}, "String must begin with quotation marks" },
/* 36 - StringMid */					{ Nothing, {Quotation, IsPrint}, {37, 38}, "String can't contain non-printable characters and must be closed with quotation marks" },
/* 37 - Quotation */					{ Nothing, {End, Default}, {39, 38}, "" },
/* 38 - Char */							{ WriteChar, {Default}, {36}, "" },
/* 39 - StringEnd */					{ WriteTerminate, {Default}, {StateAccept}, ""}
};

/* Returns the action the current state requires be run */
enum StateAction getStateAction(int currentState) {
	return States[currentState].stateAction;
}

/* Matches the conditions and returns the index of the next state, given the current state, and pointer to input */
int getNextState(int currentState, char **p_line) {
	int i;
	for (i = 0; i < MAX_CONDITIONS && States[currentState].conditions[i]; i++)
		if (States[currentState].conditions[i](p_line))
			return States[currentState].nextStates[i];
	return StateError;
}

/* Returns the defined error message for a state */
char *getStateErrorMessage(int currentState) {
	return States[currentState].errorMessage;
}
