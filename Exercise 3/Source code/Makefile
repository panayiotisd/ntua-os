CC = gcc
CFLAGS = -Wall -O2

all : pipesem-test mandel procs-shm

proc-common.o : proc-common.c
	$(CC) $(CFLAGS) -c -o proc-common.o proc-common.c

pipesem.o : pipesem.c
	$(CC) $(CFLAGS) -c -o pipesem.o pipesem.c

pipesem-test.o : pipesem-test.c
	$(CC) $(CFLAGS) -c -o pipesem-test.o pipesem-test.c

pipesem-test : pipesem.o pipesem-test.o
	$(CC) -o pipesem-test pipesem.o pipesem-test.o

mandel-lib.o : mandel-lib.c
	$(CC) $(CFLAGS) -c -o mandel-lib.o mandel-lib.c

mandel.o : mandel.c
	$(CC) $(CFLAGS) -c -o mandel.o mandel.c

mandel : pipesem.o mandel-lib.o mandel.o
	$(CC) -o mandel pipesem.o mandel-lib.o mandel.o

procs-shm.o : procs-shm.c
	$(CC) $(CFLAGS) -c -o procs-shm.o procs-shm.c

procs-shm : pipesem.o proc-common.o procs-shm.o
	$(CC) -o procs-shm pipesem.o proc-common.o procs-shm.o

clean :
	rm -f *.o pipesem-test mandel procs-shm
