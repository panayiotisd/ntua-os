#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "pipesem.h"

void pipesem_init(struct pipesem *sem,int val)
{
	int pfd[2],data=42;
	if(pipe(pfd)<0)
	{
		perror("pipesem_init: pipe");
		exit(1);
	}
	sem->rfd=pfd[0];
	sem->wfd=pfd[1];
	for(;val>0;val--)
		if(write(sem->wfd,&data,sizeof(int))<0)
		{
			perror("pipesem_init: write");
			exit(1);
		}
}

void pipesem_wait(struct pipesem *sem)
{
	int data;
	if(read(sem->rfd,&data,sizeof(int))<0)
	{
		perror("pipesem_wait: read");
		exit(1);
	}
}

void pipesem_signal(struct pipesem *sem)
{
	int data=42;
	if(write(sem->wfd,&data,sizeof(int))<0)
	{
		perror("pipesem_signal: write");
		exit(1);
	}
}

void pipesem_destroy(struct pipesem *sem)
{
	if(close(sem->rfd)<0)
	{
		perror("pipesem_destroy: close");
		exit(1);
	}
	if(close(sem->wfd)<0)
	{
		perror("pipesem_destroy: close");
		exit(1);
	}
}
