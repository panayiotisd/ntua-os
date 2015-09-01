#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <math.h>

#include "pipesem.h"
#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

// Number of processes to create
int procs_no=2;

// Output at the terminal is x_chars wide by y_chars long
int y_chars=50;
int x_chars=90;

// The part of the complex plane to be drawn:
// upper left corner is (xmin,ymax), lower right corner is (xmax,ymin)
double xmin=-1.8,xmax=1.0;
double ymin=-1.0,ymax=1.0;

// Every character in the final output is
// xstep x ystep units wide on the complex plane.
double xstep;
double ystep;

// functions
void child_proc(struct pipesem*,struct pipesem*,int);
void command_line_arguments(int,char*[]);
void compute_mandel_line(int,int[]);
void output_mandel_line(int,int[]);

int main(int argc,char *argv[])
{
	int cnt,status;
	// process command line arguments
	command_line_arguments(argc,argv);
	// calculate steps
	xstep=(xmax-xmin)/x_chars;
	ystep=(ymax-ymin)/y_chars;
	// initialize a semaphore for every child process
	struct pipesem *sems[procs_no];
	for(cnt=0;cnt<procs_no;cnt++) sems[cnt]=(struct pipesem*)malloc(sizeof(struct pipesem));
	for(cnt=0;cnt<procs_no;cnt++) pipesem_init(sems[cnt],0);
	// signal the first child process
	pipesem_signal(sems[0]);
	// fork every child process
	pid_t pids[procs_no];
	for(cnt=0;cnt<procs_no;cnt++)
	{
		pids[cnt]=fork();
		if(pids[cnt]<0)
		{
			perror("main: fork");
			exit(1);
		}
		if(!pids[cnt])
		{
			// child process
			child_proc(sems[cnt],sems[(cnt+1)%procs_no],cnt);
			exit(1);
		}
	}
	// wait for all child processes to exit
	for(cnt=0;cnt<procs_no;cnt++) wait(&status);
	// destroy the semaphores
	for(cnt=0;cnt<procs_no;cnt++) pipesem_destroy(sems[cnt]);
	// set terminal color back to default
	reset_xterm_color(1);
	return 0;
}

// Computes and outputs the mandel lines such that: line % procs_no == line_no
void child_proc(struct pipesem *sem,struct pipesem *next_sem,int line_no)
{
	int color_val[x_chars],line;
	for(line=line_no;line<y_chars;line+=procs_no)
	{
		compute_mandel_line(line,color_val);
		pipesem_wait(sem);
		output_mandel_line(STDOUT_FILENO,color_val);
		pipesem_signal(next_sem);
	}
	exit(0);
}

// Processing command line arguments given to customize the output
void command_line_arguments(int argc,char *argv[])
{
	int cnt; char opt;
	for(cnt=1;cnt<argc;cnt++)
	{
		if(strlen(argv[cnt])<3||argv[cnt][1]!='=')
		{
			printf("Invalid command line arguments: Wrong syntax\n");
			printf("Check syntax: [option_character]=[value_to_set]\n");
			exit(1);
		}
		opt=argv[cnt][0];
		argv[cnt][0]=argv[cnt][1]=' ';
		switch(opt)
		{
			case 'n': procs_no=atoi(argv[cnt]); break;
			case 'N': procs_no=atoi(argv[cnt]); break;
			case 'w': x_chars=atoi(argv[cnt]); break;
			case 'W': x_chars=atoi(argv[cnt]); break;
			case 'l': y_chars=atoi(argv[cnt]); break;
			case 'L': y_chars=atoi(argv[cnt]); break;
			case 'x': xmin=atof(argv[cnt]); break;
			case 'X': xmax=atof(argv[cnt]); break;
			case 'y': ymin=atof(argv[cnt]); break;
			case 'Y': ymax=atof(argv[cnt]); break;
			default : printf("Invalid command line arguments: Invalid option character\n");
					  printf("Check options: n/N, w/W, l/L, x,X, y,Y\n");
					  exit(1);
		}
	}
	printf("\nProcesses(n/N): %d\nScreen(w/W*l/L): %d*%d\n",procs_no,x_chars,y_chars);
	printf("Plot([x,X]*[y,Y]): [%.3f,%.3f]*[%.3f,%.3f]\n\n\n",xmin,xmax,ymin,ymax);
	if(procs_no<=0||x_chars<=0||y_chars<=0||xmin>=xmax||ymin>=ymax)
	{
		printf("Invalid command line arguments: Wrong values\n");
		printf("Check values: N>0, W>0, L>0, x<X, y<Y\n");
		exit(1);
	}
}

// This function computes a line of output
// as an array of x_char color values.
void compute_mandel_line(int line,int color_val[])
{
	// x and y traverse the complex plane.
	double x,y;
	int n,val;
	// Find out the y value corresponding to this line
	y=ymax-ystep*line;
	// and iterate for all points on this line
	for(x=xmin,n=0;x<=xmax;x+=xstep,n++)
	{
		// Compute the point's color value
		val=mandel_iterations_at_point(x,y,MANDEL_MAX_ITERATION);
		if(val>255) val=255;
		// And store it in the color_val[] array
		val=xterm_color(val);
		color_val[n]=val;
	}
}

// This function outputs an array of x_char color values
// to a 256-color xterm.
void output_mandel_line(int fd,int color_val[])
{
	int i;
	char point='@';
	//char point[3]={0xe2,0x96,0x88};
	char newline='\n';
	for(i=0;i<x_chars;i++)
	{
		// Set the current color, then output the point
		set_xterm_color(fd,color_val[i]);
		if(write(fd,&point,1)!=1)
		{
			perror("output_mandel_line: write point");
			exit(1);
		}
	}
	// Now that the line is done, output a newline character
	if(write(fd,&newline,1)!=1)
	{
		perror("output_mandel_line: write newline");
		exit(1);
	}
}
