#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "proc-common.h"
#include "tree.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

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
	sleep(SLEEP_TREE_SEC);
	// print the process tree from root
	show_pstree(pid);
	// wait for root process to terminate
	pid=wait(&status);
	explain_wait_status(pid,status);
	return 0;
}

void fork_procs(struct tree_node *node)
{
	pid_t chpid,pid[node->nr_children];
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
	// wait for every child to terminate
	for(cnt=0;cnt<node->nr_children;cnt++)
	{
		printf("%s(%ld): Waiting(%d/%d)...\n",node->name,(long)getpid(),cnt+1,node->nr_children);
		chpid=wait(&status);
		explain_wait_status(chpid,status);
	}
	// sleep processes sleep before exiting
	if(!(node->nr_children))
	{
		printf("%s(%ld): Sleeping...\n",node->name,(long)getpid());
		sleep(SLEEP_PROC_SEC);
	}
	printf("%s(%ld): Exiting...\n",node->name,(long)getpid());
	exit(0);
}
