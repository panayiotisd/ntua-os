#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#include "proc-common.h"
#include "tree.h"

void fork_procs(struct tree_node*,int*);
int compute(struct tree_node*,int*);

int main(int argc,char *argv[])
{
	struct tree_node *root;
	pid_t pid;
	int pfd[2],status,res;
	if(argc!=2)
	{
		fprintf(stderr,"Usage: %s <input_tree_file>\n",argv[0]);
		exit(1);
	}
	// scan tree_nodes
	root=get_tree_from_file(argv[1]);
	// print tree_nodes
	print_tree(root);
	// create pipe
	printf("main(%ld): Creating pipe...\n",(long)getpid());
	if(pipe(pfd)<0)
	{
		perror("main: pipe");
		exit(1);
	}
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
		fork_procs(root,pfd);
		exit(1);
	}
	// lock write for parent
	if(close(pfd[1])<0)
	{
		perror("main: close");
		exit(1);
	}
	// wait for root process to terminate
	pid=wait(&status);
	explain_wait_status(pid,status);
	// read result from pipe
	if(read(pfd[0],&res,sizeof(res))!=sizeof(res))
	{
		perror("main: read");
		exit(1);
	}
	printf("Result: %d\n",res);
	return 0;
}

void fork_procs(struct tree_node *node,int *pfd)
{
	pid_t chpid,pid[node->nr_children];
	int cnt,res,val[node->nr_children],status,pfds[node->nr_children][2];
	printf("%s(%ld): Starting...\n",node->name,(long)getpid());
	change_pname(node->name);
	// lock read for child
	if(close(pfd[0])<0)
	{
		perror("fork_procs: close");
		exit(1);
	}
	// create a pipe for every child
	for(cnt=0;cnt<node->nr_children;cnt++)
	{
		printf("%s(%ld): Creating pipes(%d/%d)...\n",node->name,(long)getpid(),cnt+1,node->nr_children);
		if(pipe(pfds[cnt])<0)
		{
			perror("fork_procs: pipe");
			exit(1);
		}
	}
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
			fork_procs(node->children+cnt,pfds[cnt]);
			exit(1);
		}
	}
	// lock write for parent
	for(cnt=0;cnt<node->nr_children;cnt++)
	{
		if(close(pfds[cnt][1])<0)
		{
			perror("fork_procs: close");
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
	// read result from child pipes for every child
	for(cnt=0;cnt<node->nr_children;cnt++)
	{
		printf("%s(%ld): Reading pipes(%d/%d)...\n",node->name,(long)getpid(),cnt+1,node->nr_children);
		if(read(pfds[cnt][0],val+cnt,sizeof(val[cnt]))!=sizeof(val[cnt]))
		{
			perror("fork_procs: read");
			exit(1);
		}
	}
	// compute node result
	res=compute(node,val);
	// write result to parent pipe
	printf("%s(%ld): Writing pipe...\n",node->name,(long)getpid());
	if(write(pfd[1],&res,sizeof(res))!=sizeof(res))
	{
		perror("fork_procs: write");
		exit(1);
	}
	printf("%s(%ld): Exiting...\n",node->name,(long)getpid());
	exit(0);
}

int compute(struct tree_node *node,int *val)
{
	int res,cnt;
	if(!(node->nr_children)) return atoi(node->name);
	if(!strcmp(node->name,"+"))
	{
		for(res=0,cnt=0;cnt<node->nr_children;cnt++) res+=val[cnt];
		return res;
	}
	if(!strcmp(node->name,"*"))
	{
		for(res=1,cnt=0;cnt<node->nr_children;cnt++) res*=val[cnt];
		return res;
	}
	fprintf(stderr,"Unknown operator: \"%s\"\n",node->name);
	exit(1);
}
