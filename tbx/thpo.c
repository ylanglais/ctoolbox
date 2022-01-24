#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "mem.h"
#include "err.h"
#include "queue.h"
#include "storage.h"
#include "tstmr.h"

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	30/05/2016  0.5  creation
	02/06/2016  1.0  initial version
	04/08/2016  1.1  add changelog
	23/02/2018  1.2  fix over from w/ thpo_data_status_txt & thpo_status_txt
*/   

char thpo_MODULE[]  = "Thread pool";
char thpo_PURPOSE[] = "Allow parallel processing of data throught a pool of threads";
char thpo_VERSION[] = "1.2";
char thpo_DATEVER[] = "23/02/2016";

extern int errno;

char *thpo_status_txt[] = {
	"none",
	"created",
	"ready",
	"started",
	"running",
	"processed",
	"invalid status"
};

char *thpo_data_status_txt[] = {
	"wait",
	"done",
	"stop",
	"invalid data status"
};

typedef struct {
	pthread_t  	   thrid;
	int			   tnum;
	tstamp_t       start;
	tstamp_t 	   end;
	int			   ndata;
} thpo_stats_t, *pthpo_stats_t;

typedef struct {
	unsigned long  dataid;
	void          *data;
	int 		   tnum;
	pthread_t      thrid;
	tstamp_t       start;
	tstamp_t       end;
	unsigned long  wait;
	int			   ret;
} thpo_data_t, *pthpo_data_t;
	
typedef struct {
	int status;
	int data_status;
	int tnum;
	int (*worker_cb)(void *);
	pthread_t     *tids;
	pqueue_t      inq;
	// pqueue_t      ouq;
	pstorage_t    data;
	thpo_stats_t *stats;
	tstamp_t start;
	tstamp_t end;
} thpo_t, *pthpo_t;

#define  _thpo_c_
#include "thpo.h"
#undef   _thpo_c_

static char *
thpo_status_string(int status) {
	if (status < thpo_NONE || status >= thpo_INVALID) 
		return thpo_status_txt[thpo_INVALID];
	return thpo_status_txt[status];
}

static char *
thpo_status_data_string(int status) {
	if (status < thpo_DATA_WAIT || status >= thpo_DATA_INVALID) 
		return thpo_data_status_txt[thpo_DATA_INVALID];
	return thpo_data_status_txt[status];
}

pthpo_t
thpo_destroy(pthpo_t tp) {
	if (!tp) return NULL;
	if (tp->tids)  tp->tids  = mem_free(tp->tids);
	if (tp->inq)   tp->inq   = queue_destroy(tp->inq);
 // if (tp->ouq)   tp->ouq   = queue_destroy(tp->ouq); 
	if (tp->data)  tp->data  = storage_destroy(tp->data);
	if (tp->stats) tp->stats = mem_free(tp->stats);
	tp = mem_free(tp);
	return NULL;
}

pthpo_t
thpo_new(int nthreads, int (*worker_cb)(void *)) {
	pthpo_t tp;

	if (nthreads < 1) {
		return NULL;
	}
	if (!(tp = mem_zmalloc(sizeof(thpo_t)))) {
		err_error("cannot create thread pool");
		return NULL;	
	}
	
	if (!(tp->tids  = mem_zmalloc(nthreads * sizeof(pthread_t)))) {
		err_error("cannot allocate threads");
		return thpo_destroy(tp);
	}

	if (!(tp->inq   = queue_new())) {
		err_error("cannot create input queue");
		return thpo_destroy(tp);
	}

	if (!(tp->data  = storage_new(sizeof(thpo_data_t), 100))) {
		err_error("cannot create storage for thread data");
		return thpo_destroy(tp);
	}

	if (!(tp->stats = mem_zmalloc(nthreads * sizeof(thpo_stats_t)))) {
		err_error("cannot allocate memory for statistics");
		return thpo_destroy(tp);
	}

	tp->tnum        = nthreads;
	tp->worker_cb   = worker_cb;
	tp->status      = thpo_CREATED;
	tp->data_status = thpo_DATA_WAIT;

	return tp;
}

static void *
_thpo_worker(void *data) {
	int i;
	int tn = -1;
	unsigned long this;
	pthpo_t tp;
	pthpo_stats_t st;
	pthpo_data_t tdat;

	if (!data) {
		err_error("empty data");
		return (void *) 1;
	}

	tp   = (pthpo_t) data;

	this = pthread_self();
	err_debug("entered thread %u", this);

	/* Make sure all data is actually availible in tp after thread launch: */
	while (tp->status != thpo_RUNNING) usleep(50);

	if (tp->stats) {
		for (i = 0; i < tp->tnum; i++) {
			if (tp->stats[i].thrid == this) {
				tn = i;
				break;
			}	
		}
		err_debug("thread %u has seq %d", this, tn);
	}
	if (tn < 0) {
		err_error("current thread %u not referenced, returning", this);
		return (void *) 2;
	}  

	st = tp->stats + i;
	st->tnum  = tn;
	st->start = tstamp_get();
	err_debug("thread %d (%u) started", tn, this);
	
	do {
		while (tp->data_status != thpo_DATA_STOP && (tdat = (pthpo_data_t) queue_pop(tp->inq))) {
			st->ndata++;
			err_debug("thread %d (%u) runs data batch %d (data id: %u, ptr: %p)", tn, this, st->ndata, tdat->dataid, tdat->data);
			tdat->thrid = this;
			tdat->tnum  = tn;
			tdat->start = tstamp_get();
			tdat->ret   = tp->worker_cb(tdat->data);
			tdat->end   = tstamp_get();	
			err_debug("thread %d (%u) data batch %d (data id: %u) ended with status", tn, this, st->ndata, tdat->dataid, tdat->ret);
		}
		if (tp->data_status != thpo_DATA_DONE && tp->data_status != thpo_DATA_STOP) {
			usleep(50);
			err_debug("thread %d: thread pool status = %s, thread pool data_status = '%s'", tn, 
				thpo_status_string(tp->status), thpo_status_data_string(tp->data_status));
		}
	} while (tp->data_status != thpo_DATA_DONE && tp->data_status != thpo_DATA_STOP);

	st->end = tstamp_get();
	err_debug("thread %d (%u) ended after %d batch of user data", tn, this, st->ndata);
	return NULL;
}

int
thpo_data_status(pthpo_t tp) {
	if (!tp) return 0;
	return tp->data_status;
}

int 
thpo_status(pthpo_t tp) {
	if (!tp) return 0;
	return tp->data_status;
}

int 
thpo_push(pthpo_t tp, unsigned long dataid, void *data) {
	thpo_data_t d;
	pthpo_data_t p;

	if (!tp) {
		err_warning("invalid thread pool discarding data %u @ %p", dataid, data);
		return -1;
	}
	if (!data) {
		err_warning("no data passed");
		return -2;
	}

	d.dataid = dataid;
	d.data   = data;

	p = (pthpo_data_t) storage_add(tp->data, (char *) &d);
	queue_push(tp->inq, (void *) p);

	if (tp->status == thpo_CREATED) tp->status = thpo_READY;
	
	return 0;
}

int
thpo_run(pthpo_t tp) {
	int i;
	if (!tp) {
		err_error("Invalid thread pool");
		return 1;
	}

	tp->status = thpo_STARTED;
	tp->start = tstamp_get();
	for (i = 0; i < tp->tnum; i++) {
		int r;
		if ((r = pthread_create((tp->tids) + i, NULL, _thpo_worker, tp))) {
			err_error("cannot create thread (code %d) reason: %s\n", r, strerror(errno));
		} 
		tp->stats[i].thrid = tp->tids[i];
	}
	tp->status = thpo_RUNNING;
	return 0;
}

int thpo_data_stop(pthpo_t tp) {
	if (!tp) return 1;
	tp->data_status = thpo_DATA_STOP;
	return 0;
}

int thpo_data_done(pthpo_t tp) {
	if (!tp) return 1;
	tp->data_status = thpo_DATA_DONE;
	return 0;
}

int thpo_done(pthpo_t tp) {
	int i;
	if (!tp) return 1;
	thpo_data_done(tp);
	for (i = 0; i < tp->tnum; i++) {
		void *retp;
		if (tp->tids[i]) {
			pthread_join(tp->tids[i], &retp);
		} else {
			err_warning("thread %d is not defined\n", i);
		}
	}
	tp->end = tstamp_get();
	tp->status = thpo_PROCESSED;
	return 0;
}

int
thpo_run_blocking(pthpo_t tp) {
	if (tp->status != thpo_READY) {
		err_warning("Thread pool cannot start processing since status not READY but %s", thpo_status_string(tp->status));
		return 2;
	}
	thpo_run(tp);
	return thpo_done(tp);
}

#if 0
tstamp_t 
thpo_processing_time(thpo_t tp, unsigned long dataid) {
	int i, n;
	if (!tp) {
		err_warning("invalid thread pool");
		return tstamp_zero();
	}
	if (tp->status != thpo_PROCESSED) {
		err_warning("thread pool has not finished processing (current status is %s)", thpo_status_string(tp->status));
		return tstamp_zero();
	}
	n = storage_used(tp->data);
	for (i = 0; i < n; i++) {
		pthpo_data_t d;
		d = (pthpo_data_t) storage_get(tp->data, i);
		if (d->dataid == dataid) return tstmr_sub(d->end, d->start);
	} 
	return tstamp_zero();
}
#endif

void 
thpo_stats_dump(pthpo_t tp) {
	int i, ndata;
	char b[60];
	pthpo_data_t tdat;
	tstamp_t total;

	if (!tp) return;
	
	printf("\nThread pool (%s) is composed of %d threads during %s.\n", thpo_status_string(tp->status), tp->tnum, 
			tstamp_duration_fmt(b, tstamp_sub(tp->end, tp->start)));

	printf("Thread details:\n");
	for (i = 0; i < tp->tnum; i++) {
		printf("\tThread %d (%u) processed %d batches in %s\n", tp->stats[i].tnum, tp->stats[i].thrid, tp->stats[i].ndata, 
			tstamp_duration_fmt(b, tstamp_sub(tp->stats[i].end, tp->stats[i].start)));
	}

	total = tstamp_zero();
	ndata = storage_used(tp->data);

	for (i = 0; i < ndata; i++) {
		tstamp_t elapsed;
		tdat    = (pthpo_data_t) storage_get(tp->data, i);
		elapsed = tstamp_sub(tdat->end, tdat->start); 
		total   = tstamp_add(total, elapsed);
	}
	printf("Total processing time: %s\n\n", tstamp_duration_fmt(b, total));
}

void
thpo_stats_detail(pthpo_t tp) {
	int i, ndata;
	char b[60];
	pthpo_data_t tdat;
	tstamp_t total;

	if (!tp) return;
	
	printf("\nThread pool (%s) is composed of %d threads during %s.\n", thpo_status_string(tp->status), tp->tnum, 
			tstamp_duration_fmt(b, tstamp_sub(tp->end, tp->start)));

	printf("Thread details:\n");
	for (i = 0; i < tp->tnum; i++) {
		printf("\tThread %d (%u) processed %d batches in %s\n", tp->stats[i].tnum, tp->stats[i].thrid, tp->stats[i].ndata, 
			tstamp_duration_fmt(b, tstamp_sub(tp->stats[i].end, tp->stats[i].start)));
	}

	total = tstamp_zero();
	ndata = storage_used(tp->data);

	printf("Batch processing details:\n");
	for (i = 0; i < ndata; i++) {
		char B[60];
		tstamp_t elapsed;
		tdat    = (pthpo_data_t) storage_get(tp->data, i);
		elapsed = tstamp_sub(tdat->end, tdat->start); 
		total   = tstamp_add(total, elapsed);
		printf("\tData batch %d (%p) processed by thread %d (%u) returned %d after %s\n",
			tdat->dataid, tdat->data, tdat->tnum, tdat->thrid, tdat->ret, tstamp_duration_fmt(B, elapsed));
	}
	printf("Total processing time: %s\n\n", tstamp_duration_fmt(b, total));
}

#ifdef _test_thpo_
#include <stdlib.h>

#include "list.h"
#include "tstmr.h"

int 
workr(void *p) {
	int i;

	i = (int)(unsigned long) p;
	if (i < 0) return 1;
	printf("sleep(%d)\n", i);
	sleep(i);
	return 0;
}

#define echo_push_sum(v) {printf("push %d\n", v); thpo_push(tp, i++, (void *) v); total += v;}

int main(int n, char *a[]) {
	int nth = 5, total = 0, i = 0;
	pthpo_t tp;

	err_init(NULL, err_DEBUG);

	if (n > 1) nth = atoi(a[1]);

	tp = thpo_new(nth, workr);

	printf("run pool to sleep %ds:\n", total);
	thpo_run(tp);

	printf("create and populate list\n");
	echo_push_sum(1);
	echo_push_sum(2);
	echo_push_sum(3);
	echo_push_sum(2);
	echo_push_sum(5);
	echo_push_sum(-1);
	echo_push_sum(2);
	echo_push_sum(4);
	echo_push_sum(3);
	echo_push_sum(4);
	echo_push_sum(2);
	echo_push_sum(1);

	thpo_done(tp);
	thpo_stats_dump(tp);
	thpo_stats_detail(tp);
	thpo_destroy(tp);
	
	return 0;
}
#endif 
