#ifndef GRAMMAR
#define GRAMMAR

#include <stdio.h>

#define MAX_LINE 80
#define MAX_LABEL 31

enum ParseMode {PARSE_SYMBOLS, PARSE_ALL};

int parse(FILE *stream, enum ParseMode m);

#endif
