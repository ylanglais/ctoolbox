/*
	This code is released under GPL terms. 
	Please read license terms and conditions at http://www.gnu.org/

	Yann LANGLAIS

	Changelog:
	04/09/2001  2.0 fixes, use of the new memory allocator, ...
	11/03/2002	2.5 Make code reentrant w/ pthread_mutexes 	
	08/04/2004	3.0 reentrant fixes, interface cleaning & commenting.
*/

#include <errno.h>
#include <string.h>
#include <pthread.h>

#include "mem.h"
#include "err.h"
#include "sthpo.h"

extern int errno;

char sthpo_MODULE[]  = "Simple thread pool";
char sthpo_PURPOSE[] = "Allow parallel processing of data throught a pool of threads";
char sthpo_VERSION[] = "1.0";
char sthpo_DATEVER[] = "30/05/2016";

int
sthpo_runpool(int tnum, void *(*worker)(void *), void *userdata) {
	pthread_t *tids;
	int i;
	void *retp;

	if (!(tids = mem_zmalloc(tnum * sizeof(pthread_t)))) {
		return 1;
	}

	for (i = 0; i < tnum; i++) {
		int r;
		if ((r = pthread_create(tids+i, NULL, worker, userdata))) {
			err_error("cannot create thread (code %d) reason: %s\n", r, strerror(errno));
		} 
	}
	for (i = 0; i < tnum; i++) {
		if (tids[i]) {
			pthread_join(tids[i], &retp);
		} else {
			err_warning("thread %d is not defined\n", i);
		}
	}
	if (tids) free(tids);
	return 0;
}

#ifdef _test_sthpo_
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "list.h"
#include "tstmr.h"

void *
workr(void *p) {
	int i, n;
	char b[50], e[100];
	tstamp_t sta, end;

	n = 0;
	printf("%s: thread %u started\n", tstamp_fmt(b, sta = tstamp_get()), pthread_self());

	while ((i = (int)(unsigned long) list_pop_first((plist_t) p))) {
		n++;
		tstamp_t ts, te;
		printf("%s: %u run sleep(%i)\n", tstamp_fmt(b, ts = tstamp_get()), pthread_self(), i);
		sleep(i);
		te = tstamp_get();
		printf("%s: %u slept(%d), elapsed = %s\n", tstamp_fmt(b, te), pthread_self(), i, tstamp_duration_fmt(e, tstamp_sub(te, ts)));
	}

	end = tstamp_get();
	printf("%s: %u processed %d elements in %s and is now closing\n", tstamp_fmt(b, end), pthread_self(), n, tstamp_duration_fmt(e, tstamp_sub(end, sta)));
	return NULL;
}

#define echo_push_sum(v) {printf("push %d\n", v); list_push(l, (void *) v); total += v; }

int main(int n, char *a[]) {
	int total = 0;
	int nth = 5;
	plist_t l;
	tstamp_t sta, end;
	char b[100];

	if (n > 1) nth = atoi(a[1]);

	printf("create and populate list\n");
	l = list_new();
	echo_push_sum(1);
	echo_push_sum(2);
	echo_push_sum(3);
	echo_push_sum(2);
	echo_push_sum(5);
	echo_push_sum(2);
	echo_push_sum(4);
	echo_push_sum(3);
	echo_push_sum(4);
	echo_push_sum(2);
	echo_push_sum(1);

	sta = tstamp_get();
	printf("run pool to sleep %ds:\n", total);
	sthpo_runpool(nth, workr, (void *) l);

	end = tstamp_get();
	printf("\n***\nslept %ds in %s unsing %d threads\n", total, tstamp_duration_fmt(b, tstamp_sub(end, sta)), nth);
	
	return 0;
}
#endif 
