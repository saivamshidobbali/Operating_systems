CC=gcc

CFLAGS= -g -pthread -O2 -fmessage-length=0 -pedantic-errors -std=gnu99 -Werror -Wall -Wextra -Wwrite-strings -Winit-self -Wcast-align -Wcast-qual -Wpointer-arith -Wstrict-aliasing -Wformat=2 -Wmissing-include-dirs -Wno-unused-parameter -Wshadow -Wuninitialized -Wold-style-definition

program1: program_1.c program2
	$(CC) $(CFLAGS) -o program_1 program_1.c

program2: program_2.c program3
	$(CC) $(CFLAGS) -o program_2 program_2.c

program3: program_3.c 
	$(CC) $(CFLAGS) -o program_3 program_3.c

