# Assembler-Project for Course in Operating System Programming

##### Usage:
1. Make the assembler with the provided makefile.
2. Run the created file `assembler`, with input files ('.as' file extension). Multiple source files may be passed as arguments.
3. If a file has syntax errors, the assembler will print an error message for that file and will not generate output.
4. Per valid input file, up to three output files may be created (with same file name, differing in extension):
    * An assembled output in hex ('.ob' file extension) - Always generated
    * A file consisting of external labels used in source ('.ext' file extension)
    * A file of entry labels defined in the source('.ent' file extension)


##### An example for input an output can be found in the `example` directory
