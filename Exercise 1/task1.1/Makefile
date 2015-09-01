CC = gcc -std=c99
CF = -Wall -O3

zing : main.o zing.o
	$(CC) main.o zing.o -o zing

zing2 : main.o zing2.o
	$(CC) main.o zing2.o -o zing2

zing2.o : zing2.c
	$(CC) $(CF) -c zing2.c

main.o : main.c
	$(CC) $(CF) -c main.c

clean :
	rm -f main.o zing2.o
