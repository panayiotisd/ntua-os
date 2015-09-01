#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"
#include "tree.h"

void fork_procs(struct tree_node*);

int main(int argc,char *argv[])
{
	struct tree_node *root;
	pid_t pid;
	int status;
	if(argc!=2)
	{
		fprintf(stderr,"Usage: %s <input_tree_file>\n",argv[0]);
		exit(1);
	}
	// scan tree_nodes
	root=get_tree_from_file(argv[1]);
	// print tree_nodes
	print_tree(root);
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
		fork_procs(root);
		exit(1);
	}
	// wait for root process to stop
	wait_for_ready_children(1);
	// print the process tree from root
	show_pstree(pid);
	// activate root process
	kill(pid,SIGCONT);
	// wait for root process to terminate
	wait(&status);
	explain_wait_status(pid,status);
	return 0;
}

void fork_procs(struct tree_node *node)
{
	pid_t pid[node->nr_children];
	int cnt,status;
	printf("%s(%ld): Starting...\n",node->name,(long)getpid());
	change_pname(node->name);
	// fork every child of the node
	for(cnt=0;cnt<node->nr_children;cnt++)
	{
		pid[cnt]=fork();
		if(pid[cnt]<0)
		{
			perror("fork_procs: fork");
			exit(1);
		}
		if(!pid[cnt])
		{
			// child process
			fork_procs(node->children+cnt);
			exit(1);
		}
	}
	// wait for every child to stop
	wait_for_ready_children(node->nr_children);
	// self suspend process
	raise(SIGSTOP);
	printf("%s(%ld): Just woke up!\n",node->name,(long)getpid());
	// wake up every child
	for(cnt=0;cnt<node->nr_children;cnt++)
	{
		kill(pid[cnt],SIGCONT);
		// wait for child to terminate (DFS)
		printf("%s(%ld): Waiting(%d/%d)...\n",node->name,(long)getpid(),cnt+1,node->nr_children);
		wait(&status);
		explain_wait_status(pid[cnt],status);
	}
	printf("%s(%ld): Exiting...\n",node->name,(long)getpid());
	exit(0);
}
