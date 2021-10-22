#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IN_EXTENS ".as"

static int filenameLength; /* Current filename length, excluding extension */

/* Extracts the filename with no extension from the full filename. Returns non-zero if the filename does not have the right extension. */
int validateFilename(const char *filename) {
	char *extension;
	if ((extension = strrchr(filename, IN_EXTENS[0])) && !strcmp(extension, IN_EXTENS)) {
		filenameLength = extension - filename;
		return 0;
	}
	printf("Error: '%s' does not have '%s' extension\n", filename, IN_EXTENS);
	return 1;
}

/* Opens a file with the specified extension */
FILE *createOutFile(const char *filename, const char *extension) {
	FILE *f = NULL;
	char *tmp;
	if ((tmp = (char *) malloc(filenameLength + strlen(extension) + 1))) {
		strncpy(tmp, filename, filenameLength); /* Copy filename excluding extension */
		strcpy(tmp + filenameLength, extension); /* Copy extension to end of filename */
		if (!(f = fopen(tmp, "w")))
			printf("Error: Could not create output file '%s'\n", tmp);
		free(tmp);
	}
	else
		printf("Warn: Could not allocate required memory\n");
	return f;
}
