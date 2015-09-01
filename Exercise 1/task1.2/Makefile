CC = gcc -std=c99
CF = -Wall -O3

fconc : fconc.o
	$(CC) fconc.o -o fconc

fconc2 : fconc2.o
	$(CC) fconc2.o -o fconc2

fconc.o : fconc.c
	$(CC) $(CF) -c fconc.c

fconc2.o : fconc2.c
	$(CC) $(CF) -c fconc2.c

clean :
	rm -f fconc.o fconc2.o
