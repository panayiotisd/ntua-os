#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>

#include "pipesem.h"
#include "proc-common.h"

// This is a pointer to a shared memory area.
// It holds an integer value, that is manipulated
// concurrently by all children processes.
int *shared_memory;
// semaphores
struct pipesem *sems[3];

// functions
void proc_A(void);
void proc_B(void);
void proc_C(void);

// Use a NULL-terminated array of pointers to functions.
// Each child process gets to call a different pointer.
typedef void proc_fn_t(void);
static proc_fn_t *proc_funcs[]={proc_A,proc_B,proc_C,NULL};

int main(int argc,char *argv[])
{
	int i,status;
	pid_t pid;
	proc_fn_t *proc_fn;
	// initialize a semaphore for every process
	for(i=0;i<3;i++) sems[i]=(struct pipesem*)malloc(sizeof(struct pipesem));
	for(i=0;i<3;i++) pipesem_init(sems[i],0);
	// create a shared memory area
	shared_memory=create_shared_memory_area(sizeof(int));
	*shared_memory=1;
	// signal the print process to start the cycle
	for(i=0;i<3;i++) pipesem_signal(sems[2]);
	// fork all three processes
	for(i=0;(proc_fn=proc_funcs[i])!=NULL;i++)
	{
		printf("%lu fork()\n",(unsigned long)getpid());
		pid=fork();
		if(pid<0)
		{
			perror("parent: fork");
			exit(1);
		}
		if(pid) continue;
		// child process
		proc_fn();
		exit(1);
	}
	// parent waits for all children to terminate
	for(i=0;i<3;i++) wait(&status);
	// destroy the semaphores
	for(i=0;i<3;i++) pipesem_destroy(sems[i]);
	return 0;
}

// Proc A: n = n + 1
void proc_A(void)
{
	volatile int *n=&shared_memory[0];
	for(;;)
	{
		pipesem_wait(sems[0]);
		*n=*n+1;
		pipesem_signal(sems[2]);
	}
	exit(1);
}

// Proc B: n = n - 2
void proc_B(void)
{
	volatile int *n=&shared_memory[0];
	for(;;)
	{
		pipesem_wait(sems[1]);
		*n=*n-2;
		pipesem_signal(sems[2]);
	}
	exit(1);
}

// Proc C: print n
void proc_C(void)
{
	int val,i;
	volatile int *n=&shared_memory[0];
	for(;;)
	{
		for(i=0;i<3;i++) pipesem_wait(sems[2]);
		val=*n;
		printf("Proc C: n = %d\n",val);
		if(val!=1) printf("\t...Aaaaargh!\n");
		for(i=0;i<2;i++) pipesem_signal(sems[0]);
		pipesem_signal(sems[1]);
	}
	exit(1);
}
