#ifndef MEMIMG
#define MEMIMG

#define CODE_START 100

enum Images {CODE_IMAGE = 0, DATA_IMAGE = 1};
enum Type {BYTE = 1, HALF = 2, WORD = 4};

/* Increase initial size of memory image */
void imageExtend(enum Images imageNumber, enum Type size);

/* Return current size of memory image */
int imageSize(enum Images imageNumber);

/* Return current position in image */
int imageCurrent(enum Images imageNumber);

/* Write 'size' bytes from the long 'from' to the memory image */
void imageWriteBytes(enum Images imageNumber, long from, int size);

/* Get the byte from 'imageNumber' on index 'i' */
unsigned char imageGetByte(enum Images imageNumber, int i);

#endif
