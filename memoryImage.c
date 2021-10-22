#include <stdlib.h>

#include "memoryImage.h"

#define CHAR_BIT 8

static struct MemoryImage {
    unsigned char *image; /* Image array */
    unsigned char *pos; /* Pointer to current position in image */
    int size; /* Size of image */
} memoryImage[2];

/* Increase initial size of memory image */
void imageExtend(enum Images imageNumber, enum Type size) {
	memoryImage[imageNumber].size += size;
}

/* Return current size of memory image */
int imageSize(enum Images imageNumber) {
	return memoryImage[imageNumber].size;
}

/* Return current position in image */
int imageCurrent(enum Images imageNumber) {
	return (int) (memoryImage[imageNumber].pos - memoryImage[imageNumber].image);
}

/* Initializes image arrays with the correct sizes image count. Returns non-zero on memory failure. */
int imageAllocate(void) {
	if (memoryImage[CODE_IMAGE].size > 0 && 
		!(memoryImage[CODE_IMAGE].pos = memoryImage[CODE_IMAGE].image = (unsigned char *) malloc(memoryImage[CODE_IMAGE].size)))
		return 1;
	if (memoryImage[DATA_IMAGE].size > 0 && 
			!(memoryImage[DATA_IMAGE].pos = memoryImage[DATA_IMAGE].image = (unsigned char *) malloc(memoryImage[DATA_IMAGE].size))) {
		free(memoryImage[CODE_IMAGE].image);
		return 1;
	}
	return 0;
}

/* Deletes allocated memory in memory image */
void imageDelete(void) {
	if (memoryImage[CODE_IMAGE].image != NULL) {
		free(memoryImage[CODE_IMAGE].image);
		memoryImage[CODE_IMAGE].image = NULL;
	}
	if (memoryImage[DATA_IMAGE].image != NULL) {
		free(memoryImage[DATA_IMAGE].image);
		memoryImage[DATA_IMAGE].image = NULL;
	}
	memoryImage[CODE_IMAGE].size = memoryImage[DATA_IMAGE].size = 0;
}

/* Get the byte from 'imageNumber' on index 'i' */
unsigned char imageGetByte(enum Images imageNumber, int i) {
	return memoryImage[imageNumber].image[i];
}

/* Write 'size' bytes from the long 'from' to the memory image */
void imageWriteBytes(enum Images imageNumber, long from, int size) {
	for (; 0 < size && size <= sizeof from; size--, from >>= CHAR_BIT)
		*memoryImage[imageNumber].pos++ = from & 0xFF;
}
