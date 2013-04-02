#include "backtrace.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void *fn(void *pid_)
{
	struct bt_data *b;
	void *frames[32];
	int pid, ret, i;

	pid = atoi((const char*)pid_);
	if (pid <= 0)
		abort();

	b = bt_init(pid);
	if (!b)
		abort();

	while (1) {
		ret = bt_backtrace(b, frames, NULL,
			sizeof(frames) / sizeof(frames[0]), 0);
		if (ret < 0) {
			printf("backtrace failed.\n");
		}
		fflush(stdout);
		printf("-------------------------\n");
		for (i=0; i < ret; ++i)
			printf("  %2d. %p\n", i, frames[i]);
		fflush(stdout);
		sleep(2);
	}
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t thread;
	int i;
	if (argc <= 1) {
		printf("Usage: backtracer <PID>...\n");
		exit(1);
	}
	for (i=1; i < argc; ++i) {
		printf("Creating backtracer thread for pid %s.\n", argv[i]);
		pthread_create(&thread, NULL, fn, argv[i]);
	}
	printf("Pausing main thread.\n");
	pause();
	return 0;
}
