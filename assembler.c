#include <stdio.h>

#include "assembler.h"
#include "symbols.h"
#include "grammar.h"

int assembleFile(FILE *f);

int main(int argc, char *argv[]) {
	FILE *f;
	
	if (argc == 1) {
		printf("Error: No input files\n");
		return 0;
	}

	prepareInstructions(); /* Store language keywords in symbol table */

	while (*++argv) {
		printf((f = fopen(*argv, "r")) ? "Assembling %s:\n" : "Error: Could not open '%s'\n", *argv);
		if (f != NULL) {
			if (!validateFilename(*argv)) { /* Check input file and store it's name */
				flushBuffers(assembleFile(f), *argv); /* Assemble the file and flush the output to files if no error occurred */
				imageDelete(); /* Delete memory image */
				deleteTable(CODE | DATA | EXTERNAL | ENTRY); /* Delete the user defined symbols */
			}
			fclose(f); /* Close the assembled file */
		}
		printf("Done.\n");
	}

	deleteTable(INSTRUCTION_KEYWORD | DIRECTIVE_KEYWORD); /* Delete the keywords from the symbol table */

	return 0;
}

/* Assembles file 'f'. Returns non-zero on error. */
int assembleFile(FILE *f) {
	if (parse(f, PARSE_SYMBOLS))
		return 1;
	if (imageAllocate()) {
		printf("Error: Could not allocate required memory\n");
		return 1;
	}
	return parse(f, PARSE_ALL);
}
