#include <stdlib.h> 
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/sem.h> 
#include <string.h>
#include <errno.h>
extern int errno;

#include <tbx/err.h>

#include "slock.h"

int
slock_new(char *fname) {
	int id;
	key_t k;
	k = ftok(fname, 'S');
	if ((id = semget(k, 1, S_IRUSR | S_IWUSR | IPC_CREAT)) < 0) {
		err_error("cannot create semaphore: %s", strerror(errno));
	}
	semctl(id, 0, SETVAL, 1);

	return id;
}

int
slock_lock(int id) {
	struct sembuf s = {0, -1, 0};
	semop(id, &s, 1);
	return 0;
}

int
slock_unlock(int id) {
	struct sembuf s = {0, 1, 0};
	semop(id, &s, 1);
	return 0;
}

int slock_semval(int id) {
	return semctl(id, 0, GETVAL);
}

int
slock_destroy(int id) {
	if (id) semctl(id, 0, IPC_RMID, 0);
	return 0;
}

#ifdef _test_slock_

void
worker(int id, int count) {
	int i;
	int pid = getpid();
	
	err_log("% 5d starting", pid);
	for (i = 0; i < count; i++) {
		err_log("% 5d loop % 3d try lock (semval is %d)", pid, i, slock_semval(id));
		slock_lock(id);
		err_log("% 5d loop % 3d locked   (semval is %d)", pid, i, slock_semval(id));
		usleep(200);
		err_log("% 5d loop % 3d unlock   (semval is %d)", pid, i, slock_semval(id));
		slock_unlock(id);
		err_log("% 5d loop % 3d unlocked (semval is %d)", pid, i, slock_semval(id));
	}
	err_log("% 5d done", pid);
}

int main(int n, char *a[]) {
	int i, id, p;
	int nprcs = 5;
	int count = 5;
	
	if (n > 1) nprcs = atoi(a[1]);
	if (n > 2) count = atoi(a[2]);

	err_log("parent process create slock");
	id = slock_new(a[0]);
	err_log("parent process created slock %d", id);
	err_log("parent process launch %d workers for %d loops", nprcs, count);

	for (i = 0; i < nprcs; i++) {
		if ((p = fork()) == 0) {
			worker(id, count);
			exit(0);
		} else if (p > 0){
			err_log("parent launched woker %d with pid %d", i, p);
		} else {
			err_error("parent could not launch child %d", i);
		}
	}
	err_log("parent is waiting all children to return");
	for (i = 0; i < count; i++) wait(NULL);
	err_log("parent destoys slock");
	slock_destroy(id);
	err_log("parent destroyed slock");
	exit(0);
}
#endif
