#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include <sys/wait.h>
#include <sys/types.h>

#include "proc-common.h"
#include "request.h"

/* Compile-time parameters. */
#define SCHED_TQ_SEC 5                /* time quantum */
#define TASK_NAME_SZ 60               /* maximum size for a task's name */

/* Processes Library */
#include "queue.h"

queue q;
pid_t cur_pid;

/*
 * SIGALRM handler
 */
static void
sigalrm_handler(int signum)
{
	if (signum != SIGALRM) {
		fprintf(stderr, "Internal error: Called for signum %d, not SIGALRM\n",
			signum);
		exit(1);
	}

	printf("ALARM! %d seconds have passed.\n", SCHED_TQ_SEC);

	kill(cur_pid,SIGSTOP);
}

/* 
 * SIGCHLD handler
 */
static void
sigchld_handler(int signum)
{
	int status;
	pid_t p;
	
	if (signum != SIGCHLD) {
		fprintf(stderr, "Internal error: Called for signum %d, not SIGCHLD\n",
			signum);
		exit(1);
	}
	
	p = waitpid(-1, &status, WUNTRACED | WNOHANG);
	if (p < 0) {
		perror("waitpid");
		exit(1);
	}
		
	if (p)
	{
		explain_wait_status(p, status);
		
		if (WIFEXITED(status) || WIFSIGNALED(status))
		{
			/* A child has died */
			if (queue_isempty(&q)) {
				fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
				exit(1);
			}
			cur_pid = queue_remove(&q);
		}
	
		if (WIFSTOPPED(status))
		{
			/* A child has stopped due to SIGSTOP/SIGTSTP, etc */
			queue_add(&q,cur_pid);
			cur_pid = queue_remove(&q);
		}
		
		/* Setup the alarm again */
		if (alarm(SCHED_TQ_SEC) < 0) {
			perror("alarm");
			exit(1);
		}
		
		printf("Starting process with PID = %ld\n", (long)cur_pid);
		kill(cur_pid,SIGCONT);
	}
}

/* Install two signal handlers.
 * One for SIGCHLD, one for SIGALRM.
 * Make sure both signals are masked when one of them is running.
 */
static void
install_signal_handlers(void)
{
	sigset_t sigset;
	struct sigaction sa;
	
	sa.sa_handler = sigchld_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGCHLD);
	sigaddset(&sigset, SIGALRM);
	sa.sa_mask = sigset;
	if (sigaction(SIGCHLD, &sa, NULL) < 0) {
		perror("sigaction: sigchld");
		exit(1);
	}

	sa.sa_handler = sigalrm_handler;
	if (sigaction(SIGALRM, &sa, NULL) < 0) {
		perror("sigaction: sigalrm");
		exit(1);
	}

	/*
	 * Ignore SIGPIPE, so that write()s to pipes
	 * with no reader do not result in us being killed,
	 * and write() returns EPIPE instead.
	 */
	if (signal(SIGPIPE, SIG_IGN) < 0) {
		perror("signal: sigpipe");
		exit(1);
	}
}

int main(int argc, char *argv[])
{

	queue_init(&q);
	
	int nproc, i;
	pid_t p;
	
	/*
	 * For each of argv[1] to argv[argc - 1],
	 * create a new child process, add it to the process list.
	 */
	for (i=1;i<argc;i++) {
		p = fork();
		if (p < 0) {
			/* fork failed */
			perror("fork");
			exit(1);
		}
		if (p == 0) {
			char executable[TASK_NAME_SZ];
			strcpy(executable,argv[i]);
			char *newargv[] = { executable, NULL, NULL, NULL };
			char *newenviron[] = { NULL };

			printf("I am PID = %ld\n", (long)getpid());
			printf("About to replace myself with the executable: %s...\n", executable);

			execve(executable, newargv, newenviron);

			/* execve() only returns on error */
			perror("execve");
			exit(1);
		}
		kill(p, SIGSTOP);
		wait_for_ready_children(1);

		// Add process to our struct
		queue_add(&q,p);
	}

	nproc = argc-1; /* number of proccesses goes here */


	/* Install SIGALRM and SIGCHLD handlers. */
	install_signal_handlers();

	if (nproc == 0) {
		fprintf(stderr, "Scheduler: No tasks. Exiting...\n");
		exit(1);
	}

	queue_print(&q);
	
	/* Get first process */
	cur_pid = queue_remove(&q);

	/* Setup the alarm */
	if (alarm(SCHED_TQ_SEC) < 0) {
		perror("alarm");
		exit(1);
	}
	
	/* Start first process */
	kill(cur_pid,SIGCONT);

	/* loop forever  until we exit from inside a signal handler. */
	while (pause()) {
		/* Setup the alarm again */
		if (alarm(SCHED_TQ_SEC) < 0) {
			perror("alarm");
			exit(1);
		}
	}

	/* Unreachable */
	queue_free(&q);
	fprintf(stderr, "Internal error: Reached unreachable point\n");
	return 1;
}
