#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

int check_sig(void)
{
	sigset_t sig_pending;
	sigpending(&sig_pending);
	if (sigismember(&sig_pending, SIGTERM) || sigismember(&sig_pending, SIGINT)) {
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if (fork() != 0) {
		exit(0);
	}

	if(setsid() < 0) {
		fprintf(stderr, "setsid error:%s\n", strerror(errno));
		exit(0);
	}

	daemon(0, 0);
	umask(0027);

	while(true) {
		int n = fork();
		pid_t pid;
		fprintf(stdout, "PID:%d, PGID:%d, SID:%d\n", getpid(), getpgrp(), getsid(0));

		int stat = 0;
		if (n > 0) {
			pid_t x = wait(&stat);
			printf("child pid=%d\n", x);
			if(WIFEXITED(stat) == 0) {
				printf("child normal exited!\n");
			}
		} else if (n == 0) {
			sigset_t sig_set;
			sigemptyset(&sig_set);
			sigaddset(&sig_set, SIGTERM);
			sigaddset(&sig_set, SIGINT);
			sigprocmask(SIG_BLOCK, &sig_set, NULL);

			while(true) {
				printf("child.\n");
				sleep(1);

				if (check_sig()) {
					printf("接收到信号，运行程序要退出！!!");
					break;
				}
			}

			exit(0);
		}

		sleep(5);
	}

	return 0;
}