#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <linux/limits.h>

#include <tbx/queue.h>
#include <tbx/pal.h>
#include <tbx/err.h>
#include <tbx/tstmr.h>
#include <tbx/futl.h>
#include <tbx/crc_32.h>

#include "qe.h"

static char _nq_data_filename[]  = ".nq_data";
//static char _nq_data_datadir[]  = ".nq_fifo";

typedef enum {
	nqerr_NONE,
	nqerr_EMPTY,
	nqerr_NO_PERSISTANT_MEMORY,
	nqerr_BAD_QUEUE,
	nqerr_BAD_QUEUE_ENTRY,
	nqerr_DIRTY_QUEUE,
	nqerr_CANNOT_WRITE
} nqerr_t;

static char *nq_err_strings[] = {
	"no error",
	"entry queue",
	"no persistant memory",
	"bad queue",
	"dirty queue",
	"cannot write"
};

typedef enum {
	nq_NONE,
	nq_NEW,
	nq_INITIALIZING,
	nq_CHECKING,
	nq_DIRTY,
	nq_CLEANING,
	nq_RUNNING,
	nq_STOPPED
} nq_state_t;

static char *nq_state_strings[] = {
	"None",
	"New",
	"Initializing",
	"Checking",	
	"Dirty",
	"Cleaning",
	"Running",
	"Stopped"
};

static char qn_magic[] = "-NQ-MAGIC-HDR->0000-";

static pqueue_t _nq_eq_ = NULL;

typedef struct {
	char 		 magic[20];
	char    	 path[PATH_MAX];
	int      	 lock;
	nq_state_t	 state;
	int		 	 nservers;
	unsigned int nfiles;
} nq_t, *pnq_t;

#define _nq_c_
#include "nq.h"
#undef _nq_c_

char *
nq_state_string(nq_state_t state) {
	return nq_state_strings[state];
}

char *
nq_qfilename(char *path) {
	if (!path) return strdup(_nq_data_filename);
	char *fname;
	if (!(fname = malloc(strlen(path) + strlen(_nq_data_filename) + 1))) {
		return NULL;
	}
	sprintf(fname, "%s/%s", path, _nq_data_filename);
	return fname;
}

#if 0
char *
nq_qdatadir(char *path) {
	if (!path) return strdup(_nq_data_datadir);
	char *fname;
	if (!(fname = malloc(strlen(path) + strlen(_nq_data_datadir) + 1))) {
		return NULL;
	}
	sprintf(fname, "%s/%s", path, _nq_data_datadir);
	return fname;
}
#endif

void
nq_dump(pnq_t nq) {
	printf("path     = %s\n", nq->path);
	printf("lock     = %d\n", nq->lock);
	printf("state    = %s\n", nq_state_string(nq->state));
	printf("nservers = %d\n", nq->nservers);
	printf("nfiles   = %d\n", nq->nfiles);

	pfutl_file_info_t dir;
	int   count;

	err_debug("check messages:");
	dir = futl_dir(nq->path, "[0-9]*\\.[0-9][0-9][0-9]\\.[0-9]*[0-9]", futl_SORT_CTIME, futl_ORDER_ASC, &count);

	int i;
	for (i = 0; i < count; i++) {
		
		err_debug("--> file %d", i);		
		
		char b[100];
		printf("File %d:\n", i);
		printf("\tname: %s\n", dir[i].fname);
		printf("\tname: %lo\n", dir[i].size);
		tstamp_t s;
		s.tv_sec  = dir[i].ctime.tv_sec;
		s.tv_usec = dir[i].ctime.tv_nsec / 1000;
		printf("\tcreation: %s\n", tstamp_fmt(b, s));
	}
}

char *
nq_err_string(nqerr_t err) {
	if ((int)err < nq_NONE || (int)err > nq_STOPPED) 
		return NULL;
	return nq_err_strings[err];
}

static void _nq_lock(pnq_t nq) {
	if (nq) {
		while (nq->lock);
		nq->lock = 1;
	}
} 
static void _nq_unlock(pnq_t nq) {
	if (nq && nq->lock) nq->lock = 0;
}

nq_state_t
nq_state(pnq_t nq) {
	if (!nq) return nq_NONE;
	return nq->state;
}

static void
_nq_new(pnq_t nq, char *path) {
	strcpy(nq->magic, qn_magic);
	nq->state = nq_INITIALIZING;
	strncpy(nq->path, path, PATH_MAX - 1);
	nq->nservers = 1;
	nq->nfiles   = 0;
	nq->state    = nq_RUNNING;
	_nq_eq_      = queue_new();
	
}

int
nq_entries_load(pnq_t nq) {
	if (queue_count(_nq_eq_) == 0) {
		pfutl_file_info_t dir;
		
		int   count;
		//dname = nq_qdatadir(nq->path);
		dir = futl_dir(nq->path, "[0-9]*\\.[0-9][0-9][0-9]\\.[0-9]*\\.hdr", futl_SORT_CTIME, futl_ORDER_ASC, &count);
		int i;
		for (i = 0; i < count; i++) {	
			pqe_t qe = qe_load(dir[i].fname);
			queue_push(_nq_eq_, qe);
		}
	}
	return 0;
}

int
nq_check_files() {
	return 0;
}

pnq_t
nq_load() {
	pnq_t  q;
	
	if (!(q = (pnq_t)  pmalloc(_nq_data_filename, 0))) return NULL;
	nq_entries_load(q);

	return q;
}

#if 0
static int
_nq_check(pnq_t nq) {
	if (!nq) return nqerr_BAD_QUEUE;
	int i;
	nq->state = nq_CHECKING;
	
	for (i = 0; i < nq->nfiles; i++) {
		size_t sz = futl_size(nq->e[i].file);
		if (sz != nq->e[i].size) {
			err_error("file %s hast not the wrong size (%lu instead of %lu)", nq->e[i].file, sz, nq->e[i].size);
			nq->state = nq_DIRTY;	
		}
		unsigned long crc = futl_crc_32(nq->e[i].file);
		if (crc != nq->e[i].crc) {
			err_error("file %s has the wrong crc (%lu instead of %lu)", nq->e[i].file, crc, nq->e[i].crc);
			nq->state = nq_DIRTY;	
		}
	}
	if (nq->state == nq_DIRTY) {
		err_error("queue is DIRTY, please check & correct before relaunching");
		return nqerr_DIRTY_QUEUE;
	}
	return nqerr_NONE;
}
#endif

void
nq_queue_lock(pnq_t nq) {
	if (!nq) return;
	_nq_lock(nq);
}

void
nq_queue_unlock(pnq_t nq) {
	if (!nq) return;
	_nq_lock(nq);
}

pnq_t
nq_new(char *path) {
	pnq_t nq;
	char wd[PATH_MAX];
	
	getcwd((char *) &wd, PATH_MAX - 1);	
	if (strcmp(wd, path)) {
		err_info("changing working directory to %s", path);
	}

	if (!(nq = (pnq_t) pmalloc(_nq_data_filename, sizeof(nq_t)))) {
		return NULL;
	}

	if (strcmp(nq->magic, qn_magic)) {
		_nq_new(nq, path);
		return nq;
	} else {
		/*
		 * TODO: check and do accordingly
		 * 	queue may already be : 
		 *	- stopped
	 	 *	- unclean
	 	 *	- running
		 */

		_nq_lock(nq);	
		nq_entries_load(nq);
#if 0
		_nq_check(nq);
#endif

#if 0
		/* queue may already be : 
			- stopped
			- unclean
			- running
		*/
		if (nq->state == nq_DIRTY_QUEUE) {
			nq = nq_destroy(nq);
			err_error("Queue as been flagged DIRTY => cannot start");
			return nq_DIRTY_QUEUE;	
		}	

		if (nq->state == nq_STOPPED) {
			nq->nserver = 1;
		}	
#endif 
	}
		
	if (nq->state == nq_STOPPED) {
		nq->state = nq_RUNNING;
	}
	nq->nservers++;

	_nq_unlock(nq);
	return nq;	
}

pnq_t
nq_destroy(pnq_t nq) {
	if (!nq) return NULL;
	pfree(nq);
	return NULL;
}
#if 0
static int
_nq_queue(pnq_t nq, pqe_t qe) {
	queue_push(_nq_eq_, qe);
	nq_dump(nq);
	return nqerr_NONE;
}
#endif

int
nq_put(pnq_t nq, pqe_t qe) {
	if (!nq) {
		err_error(nq_err_string(nqerr_BAD_QUEUE));
		if (qe) {
			qe_destroy(qe);
		}
		return nqerr_BAD_QUEUE;
	}
	if (!qe) {
		err_log("empty queue entry");
		return nqerr_BAD_QUEUE_ENTRY;
	}
	err_log("Queuing message %s.%lu", qe_file(qe));
	_nq_lock(nq);
	queue_push(_nq_eq_, qe);
	err_debug("queue count = %d", queue_count(_nq_eq_));
	nq->nfiles++;
	_nq_unlock(nq);
	return nqerr_NONE;
}

int
nq_requeue(pnq_t nq, pqe_t qe) {
	err_log("Requeuing message %s.%lu", qe_file(qe));
	qe_requeue_incr(qe);
	_nq_lock(nq);
	queue_push(_nq_eq_, qe);
	nq->nfiles++;
	_nq_unlock(nq);
	return 0;
}

int
nq_get(pnq_t nq, pqe_t *qe) {
	_nq_lock(nq);
	
	err_debug("in nq_get");
	nq_dump(nq);

	if (!nq) {
		err_error(nq_err_string(nqerr_BAD_QUEUE));
		_nq_unlock(nq);
		return nqerr_BAD_QUEUE;
	}

	if (!nq->nfiles) {
		err_info(nq_err_string(nqerr_EMPTY));
		_nq_unlock(nq);
		return nqerr_EMPTY;
	}

	/* find 1st file: */

	err_debug("queue count = %d", queue_count(_nq_eq_));
		
	if (!(*qe = queue_pop(_nq_eq_))) {
		err_info("%s, but should not !!!", nq_err_string(nqerr_EMPTY));
		_nq_unlock(nq);
		return nqerr_EMPTY;
	}	

	nq->nfiles--;
	_nq_unlock(nq);
	return nqerr_NONE;
}

int
nq_got(pnq_t nq, pqe_t qe) {
	if (!qe) {
		err_error(nq_err_string(nqerr_BAD_QUEUE_ENTRY));
		return nqerr_BAD_QUEUE_ENTRY;
	}
	qe_clear(qe);
	return 0;
}

int
nq_count(pnq_t nq) {
	return 0;
}

#if 0
void
nq_dump_content() {
}
#endif
