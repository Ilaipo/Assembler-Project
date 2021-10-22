#ifndef ASSEMBLER
#define ASSEMBLER

/* Adds keywords to symbol table, so that they can't be redfined */
void prepareInstructions(void);

/* Initializes image arrays with the correct sizes image count. Returns non-zero on memory failure. */
int imageAllocate(void);

/* Deletes allocated memory in memory image */
void imageDelete(void);

/* Extracts the filename with no extension from the full filename. Returns non-zero if the filename does not have the right extension. */
int validateFilename(char *filenameFull);

/* Deletes internal buffers storing data to be written to output. If error is 0, flushes the data into the output files. */
void flushBuffers(int error, char *filename);

#endif
