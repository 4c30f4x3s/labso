#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h>

typedef enum{false,true} boolean;
#define ERROR  if (errno) {fprintf(stderr,\
			"%s:%d: PID=%5d: Error %d (%s)\n",\
			__FILE__,\
			__LINE__,\
			getpid(),\
			errno,\
			strerror(errno));}
