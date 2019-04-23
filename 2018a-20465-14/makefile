assembler: main.o first_pass.o structLabels.o struct_ext.o second_pass.o utils.o
	gcc -g -ansi -Wall -pedantic main.o first_pass.o struct_ext.o second_pass.o utils.o structLabels.o -lm -o assembler

main.o: main.c prototypes.h assembler.h ext_vars.h structs.h utils.h
	gcc -c -ansi -Wall -pedantic main.c -o main.o

first_pass.o: first_pass.c prototypes.h assembler.h ext_vars.h structs.h utils.h
	gcc -c -ansi -Wall -pedantic first_pass.c -o first_pass.o

structLabels.o: structLabels.c prototypes.h assembler.h ext_vars.h structs.h utils.h
	gcc -c -ansi -Wall -pedantic structLabels.c -o structLabels.o

utils.o: utils.c prototypes.h assembler.h ext_vars.h structs.h utils.h
	gcc -c -ansi -Wall -pedantic utils.c -o utils.o

second_pass.o: second_pass.c prototypes.h assembler.h ext_vars.h structs.h
	gcc -c -ansi -Wall -pedantic second_pass.c -o second_pass.o

struct_ext.o: struct_ext.c prototypes.h assembler.h ext_vars.h structs.h
	gcc -c -ansi -Wall -pedantic struct_ext.c -o struct_ext.o

