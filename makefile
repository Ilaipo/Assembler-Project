assembler: assembler.c assembler.h grammar.c grammar.h grammarHelper.o fileHandler.c instructions.c memoryImage.o outBuffers.c symbols.o
	gcc -ansi -Wall -pedantic assembler.c assembler.h grammar.c grammar.h grammarHelper.o fileHandler.c instructions.c memoryImage.o outBuffers.c symbols.o -o assembler
	rm *.o

grammarHelper.o: grammarHelper.c grammarHelper.h
	gcc -c -ansi -Wall -pedantic grammarHelper.c -o grammarHelper.o

symbols.o: symbols.c symbols.h
	gcc -c -ansi -Wall -pedantic symbols.c -o symbols.o

memoryImage.o: memoryImage.c memoryImage.h
	gcc -c -ansi -Wall -pedantic memoryImage.c -o memoryImage.o
