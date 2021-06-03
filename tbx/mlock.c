#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/sem.h> 
extern int errno;


#include <tbx/err.h>
#include "mlock.h"

#define mlLOCK -1
#define mlUNLK  1

int
mlock_new(int k, int nlocks) {
	int id, i;
	if ((id = semget(k, nlocks, S_IRUSR | S_IWUSR | IPC_CREAT)) < 0) {
		err_error("cannot create mlock: %s", strerror(errno));
		return 0;
	}

	// Initialize semaphores to 1 (free):
	for (i = 0; i < nlocks; i++) 
		semctl(id, i, SETVAL, 1);

	return id;
}

int
mlock_lock(int id, int i) {
	struct sembuf s;
	s.sem_num = i; 
	s.sem_op  = mlLOCK;
	s.sem_flg = 0;
	semop(id, &s, 1);
	return 0;
}

int
mlock_unlock(int id, int i) {
	struct sembuf s;
	s.sem_num = i; 
	s.sem_op  = mlUNLK;
	s.sem_flg = 0;
	semop(id, &s, 1);
	return 0;
}

int mlock_val(int id, int i) {
	return semctl(id, i, GETVAL);
}

int
mlock_destroy(int id) {
	if (id) semctl(id, 0, IPC_RMID, 0);
	return 0;
}

#ifdef _test_mlock_

#include <tbx/tstmr.h>
static int 
_lrand(int n) {
	static int i = 0;
	if (i == 0) {
		tstamp_t t;
		t = tstamp_get();
		srand((time_t) (t.tv_sec * t.tv_usec));
		i = 1;
	}
	return rand() % n;
}

void
worker(int id, int count, int nlock) {
	int i, l;
	int pid = getpid();
	
	err_log("% 5d starting", pid);
	for (i = 0; i < count; i++) {
		l = _lrand(nlock);
		err_log("% 5d loop % 3d try lock %d (semval is %d)", pid, i, l, mlock_val(id, l));
		mlock_lock(id, l);
		err_log("% 5d loop % 3d locked   %d (semval is %d)", pid, i, l, mlock_val(id, l));
		usleep((1 + _lrand(10))*1000);
		err_log("% 5d loop % 3d unlock   %d (semval is %d)", pid, i, l, mlock_val(id, l));
		mlock_unlock(id, l);
		err_log("% 5d loop % 3d unlocked %d (semval is %d)", pid, i, l, mlock_val(id, l));
	}
	err_log("% 5d done", pid);
}

int main(int n, char *a[]) {
	int i, id, p, k;
	int nprcs = 5;
	int count = 5;
	int nlock = 2;
	
	if (n > 1) nprcs = atoi(a[1]);
	if (n > 2) count = atoi(a[2]);
	if (n > 3) nlock = atoi(a[3]);

	k = ftok(a[0], 'm');

	err_log("parent process create mlock");
	if ((id = mlock_new(k, nlock)) == 0) {
		err_error("parent could not create/get mlock");
		exit(1);
	}
	err_log("parent process created mlock %d", id);
	err_log("parent process launch %d workers for %d loops with %d locks", nprcs, count, nlock);

	for (i = 0; i < nprcs; i++) {
		if ((p = fork()) == 0) {
			worker(id, count, nlock);
			exit(0);
		} else if (p > 0){
			err_log("parent launched woker %d with pid %d", i, p);
		} else {
			err_error("parent could not launch child %d", i);
		}
	}
	err_log("parent is waiting all children to return");
	for (i = 0; i < nprcs; i++) wait(NULL);
	err_log("parent destoys mlock");
	mlock_destroy(id);
	err_log("parent destroyed mlock");
	exit(0);
}
#endif
