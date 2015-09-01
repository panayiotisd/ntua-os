#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"

#define SLEEP_PROC_SEC	10
#define SLEEP_TREE_SEC	3

// Create this process tree:
//  A-+-B---D
//    `-C

void fork_procs_A();
void fork_procs_B();
void fork_procs_C();
void fork_procs_D();

int main()
{
	pid_t pid;
	int status;
	// fork root process
	pid=fork();
	if(pid<0)
	{
		perror("main: fork");
		exit(1);
	}
	if(!pid)
	{
		// child process
		fork_procs_A();
		exit(1);
	}
	sleep(SLEEP_TREE_SEC);
	// print the process tree from root
	show_pstree(pid);
	// wait for root process to terminate
	pid=wait(&status);
	explain_wait_status(pid,status);
	return 0;
}

void fork_procs_A()
{
	// initial process A
	pid_t pid,pid1,pid2;
	int status;
	printf("A(%ld): Starting...\n",(long)getpid());
	change_pname("A");
	pid1=fork();
	if(pid1<0)
	{
		perror("A: fork_1");
		exit(1);
	}
	if(!pid1)
	{
		// child process 1
		fork_procs_B();
		exit(1);
	}
	pid2=fork();
	if(pid2<0)
	{
		perror("A: fork_2");
		exit(1);
	}
	if(!pid2)
	{
		// child process 2
		fork_procs_C();
		exit(1);
	}
	printf("A(%ld): Waiting...\n",(long)getpid());
	pid=wait(&status);
	explain_wait_status(pid,status);
	printf("A(%ld): Waiting...\n",(long)getpid());
	pid=wait(&status);
	explain_wait_status(pid,status);
	printf("A(%ld): Exiting...\n",(long)getpid());
	exit(16);
}

void fork_procs_B()
{
	// process B
	pid_t pid;
	int status;
	printf("B(%ld): Starting...\n",(long)getpid());
	change_pname("B");
	pid=fork();
	if(pid<0)
	{
		perror("B: fork");
		exit(1);
	}
	if(!pid)
	{
		// child process
		fork_procs_D();
		exit(1);
	}
	printf("B(%ld): Waiting...\n",(long)getpid());
	pid=wait(&status);
	explain_wait_status(pid,status);
	printf("B(%ld): Exiting...\n",(long)getpid());
	exit(19);
}

void fork_procs_C()
{
	// process C
	printf("C(%ld): Starting...\n",(long)getpid());
	change_pname("C");
	printf("C(%ld): Sleeping...\n",(long)getpid());
	sleep(SLEEP_PROC_SEC);
	printf("C(%ld): Exiting...\n",(long)getpid());
	exit(17);
}

void fork_procs_D()
{
	// process D
	printf("D(%ld): Starting...\n",(long)getpid());
	change_pname("D");
	printf("D(%ld): Sleeping...\n",(long)getpid());
	sleep(SLEEP_PROC_SEC);
	printf("D(%ld): Exiting...\n",(long)getpid());
	exit(13);
}
