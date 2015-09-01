#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define LEN 1000

void doWrite(int,const char*,int);
void write_file(int,const char*);

int main(int argc,char *argv[])
{
	if(argc<3)
	{
		write(STDERR_FILENO,"Usage: ",7);
		write(STDERR_FILENO,argv[0],strlen(argv[0]));
		write(STDERR_FILENO," infile1 infile2 [...] [outfile (default:fconc2.out)]\n",54);
		return 1;
	}
	int fdout;
	if(argc>3)
	{
		fdout=open(argv[argc-1],O_WRONLY|O_TRUNC|O_CREAT,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if(fdout<0)
		{
			perror("Could not open output file");
			exit(1);
		}
	}
	else
	{
		argv[argc++]="fconc2.out";
		fdout=open(argv[argc-1],O_CREAT|O_WRONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if(fdout<0)
		{
			perror("Could not create output file");
			exit(1);
		}
	}
	for(int cnt=1;cnt<argc-1;cnt++)
		if(strcmp(argv[argc-1],argv[cnt])) write_file(fdout,argv[cnt]);
		else write(STDERR_FILENO,"Output file is also an input file.Data on this file lost.\n",58);
	if(close(fdout)<0)
	{
		perror("Could not close output file");
		exit(1);
	}
	return 0;
}

void doWrite(int fd,const char *buff,int len)
{
	int idx=0,wcnt;
	while(idx<len)
	{
		wcnt=write(fd,buff+idx,len-idx);
		if(wcnt<0)
		{
			perror("Could not write output file");
			if(close(fd)<0)
			{
				perror("Could not close output file");
				exit(1);
			}
			exit(1);
		}
		idx+=wcnt;
	}
	return;
}

void write_file(int fd,const char *infile)
{
	int fdin=open(infile,O_RDONLY);
	if(fdin<0)
	{
		perror("Could not open input file");
		if(close(fd)<0)
		{
			perror("Could not close output file");
			exit(1);
		}
		exit(1);
	}
	int len;
	char *buff=(char*)malloc(LEN);
	if(!buff)
	{
		write(STDERR_FILENO,"Could not allocate memory\n",26);
		if(close(fd)<0)
		{
			perror("Could not close output file");
			exit(1);
		}
		exit(1);
	}
	while((len=read(fdin,buff,LEN)))
	{
		if(len<0)
		{
			perror("Could not read input file");
			if(close(fd)<0)
			{
				perror("Could not close output file");
				exit(1);
			}
			exit(1);
		}
		doWrite(fd,buff,len);
	}
	if(close(fdin)<0)
	{
		perror("Could not close input file");
		if(close(fd)<0)
		{
			perror("Could not close output file");
			exit(1);
		}
		exit(1);
	}
	return;
}
