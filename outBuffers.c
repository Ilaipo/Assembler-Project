#include <stdio.h>
#include <stdlib.h>

#include "symbols.h"
#include "memoryImage.h"

#define OUT_EXTENS ".ob"
#define ENT_EXTENS ".ent"
#define EXT_EXTENS ".ext"

#define BYTES_PER_ROW 4

FILE *createOutFile(const char *filename, const char *extension);

static Symbol *buffers[2]; /* For ext symbols and ent symbols */

/* Deletes internal buffers storing data to be written to output. If error is 0, flushes the data into the output files. */
void flushBuffers(int error, char *filename) {
	FILE *ext, *ent, *out;
	Symbol *tmp;
	int i;

	if (!error) {
		/* Open files, write, close */
		if (buffers[0] && (ext = createOutFile(filename, EXT_EXTENS))) { /* Externals file */
			for (tmp = buffers[0]; tmp ; tmp = tmp->next)
				fprintf(ext, "%s %04d\n", tmp->name, tmp->value + CODE_START + (hasAttribute(tmp, DATA) ? imageSize(CODE_IMAGE) : 0));
			fclose(ext);
		}
		if (buffers[1] && (ent = createOutFile(filename, ENT_EXTENS))) { /* Entry file */
			for (tmp = buffers[1]; tmp ; tmp = tmp->next)
				fprintf(ent, "%s %04d\n", tmp->name, tmp->value + CODE_START + (hasAttribute(tmp, DATA) ? imageSize(CODE_IMAGE) : 0));
			fclose(ent);
		}
		if ((out = createOutFile(filename, OUT_EXTENS))) { /* Machine language output */
			fprintf(out, "%d %d", imageSize(CODE_IMAGE), imageSize(DATA_IMAGE));
			for (i = 0; i < imageSize(CODE_IMAGE) + imageSize(DATA_IMAGE); i++) {
				if (i % BYTES_PER_ROW == 0)
					fprintf(out, "\n%04d ", i + CODE_START);
				fprintf(out, "%02X ", i < imageSize(CODE_IMAGE) ? 
					imageGetByte(CODE_IMAGE, i) : imageGetByte(DATA_IMAGE, i - imageSize(CODE_IMAGE))); /* Print byte from code or data image */
			}
			fclose(out);
		}
	}

	/* Delete buffers */
	for (i = 0; i < sizeof buffers / sizeof (Symbol *); i++) {
		while ((tmp = buffers[i]) != NULL) {
			buffers[i] = buffers[i]->next;
			free(tmp);
		}
	}
}

/* Buffer a symbol to be written to output. */
void buffer(Symbol *copy) {
	Symbol *tmp;
	int bufnum = hasAttribute(copy, EXTERNAL) ? 0 : 1;
	if ((tmp = (Symbol *) malloc(sizeof(Symbol)))) {
		*tmp = *copy;
		tmp->next = buffers[bufnum];
		buffers[bufnum] = tmp;
	}
	else {
		printf("Error: Could not allocate required memory\n");
		exit(1);
	}
}
