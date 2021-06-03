#include <linux/limits.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <tbx/pal.h>
#include <tbx/err.h>
#include <tbx/tstmr.h>
#include <tbx/futl.h>
#include <tbx/crc_32.h>

#define nqe_MAX_ADDR_LEN   16
#define nqe_MAX_FNAME_LEN 256

static char _nq_data_filename[]  = ".nq_data";
static char _nq_entry_filename[] = ".nq_entries";

typedef enum {
	nqerr_NONE,
	nqerr_EMPTY,
	nqerr_NO_PERSISTANT_MEMORY,
	nqerr_BAD_QUEUE,
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

typedef struct {
	tstamp_t arrival;
	unsigned int cnt;
	size_t	 crc;
	size_t	 size;
	char 	 from[nqe_MAX_ADDR_LEN];
	char     file[nqe_MAX_FNAME_LEN]; 
} nqe_t, *pnqe_t;

static size_t nqe_sizeof() { return sizeof(nqe_t); }
 
static char qn_magic[] = "-NQ-MAGIC-HDR->0000-";

typedef struct {
	char 		 magic[20];
	char    	 path[PATH_MAX];
	int      	 lock;
	nq_state_t	 state;
	int		 	 nservers;
	unsigned int nput;
	unsigned int nputko;
	unsigned int nget;
	unsigned int ngetko;
	tstamp_t 	 oldest;
	unsigned int nfiles;
	pnqe_t        e;
} nq_t, *pnq_t;

#define _nq_c_
#include "nq.h"
#undef _nq_c_
char *
nq_state_string(nq_state_t state) {
	return nq_state_strings[state];
}
void
nq_dump(pnq_t nq) {
	printf("path     = %s\n", nq->path);
	printf("lock     = %d\n", nq->lock);
	printf("state    = %s\n", nq_state_string(nq->state));
	printf("nservers = %d\n", nq->nservers);
	printf("nfiles   = %d\n", nq->nfiles);
	printf("e        = %p\n", nq->e);
	if (!nq->e) {
		printf("no entries");
	} else {
		int i;
		char b[100];
		pnqe_t e;
		for (i = 0, e = nq->e; i < nq->nfiles; i++, e++) {
			printf("Entry %d\n", i + 1);
			printf("   arrival = %s\n",  tstamp_fmt(b, e->arrival));
			printf("   cnt     = %u\n",  e->cnt);
			printf("   crc     = %u\n",  e->crc);
			printf("   from    = %s\n",  e->from);
			printf("   file    = %s\n",  e->file);
			printf("   size    = %lu\n", e->size);
		}
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
	nq->nput     = 
	nq->nputko   = 
	nq->nget     = 
	nq->ngetko   = 0;
	nq->oldest   = tstamp_zero();
	nq->nfiles   = 0;
	nq->e        = NULL;
	nq->state    = nq_RUNNING;
}

pnq_t
nq_load() {
	pnq_t  q;
	pnqe_t e;
	size_t s;
	if (!(q = (pnq_t)  pmalloc(_nq_data_filename,  0))) return NULL;
	if (!(e = (pnqe_t) pmalloc(_nq_entry_filename, 0)))
		q->e = NULL;
	else
		q->e = e;

	return q;	
}

static int
_nqe_load(pnq_t nq) {
	if (!nq) return nqerr_BAD_QUEUE;

	if (!(nq->e = (pnqe_t) pmalloc(_nq_entry_filename, nq->nfiles * sizeof(nqe_t)))) {
		err_error("No persistant memory");
		return nqerr_NO_PERSISTANT_MEMORY;
	}
	return nqerr_NONE;
}

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
		_nqe_load(nq);
		_nq_check(nq);

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
	if (nq->e) pfree(nq->e);
	pfree(nq);
	return NULL;
}

static int
_nqe_new(pnq_t nq, tstamp_t stamp, unsigned long crc, char *fname) {
	_nq_lock(nq);
	pnqe_t nqe;

	if (!nq->e) {
		nqe = (pnqe_t) pmalloc(_nq_entry_filename, sizeof(nqe_t));
	} else {
		nqe = (pnqe_t) prealloc(nq->e, (nq->nfiles + 1) * sizeof(nqe_t));
	}
	if (!nqe) {
		err_error("cannot create new entry");
		_nq_unlock(nq);
		return nqerr_NO_PERSISTANT_MEMORY;		
	}
	nq->e = nqe;
	nqe   = nq->e + nq->nfiles;
	memset(nqe, 0, nqe_sizeof());

	nqe->crc     = crc; 
	nqe->arrival = stamp; 
	strncpy(nqe->file, fname, nqe_MAX_FNAME_LEN);
	nq->nfiles++;
	nq_dump(nq);
	_nq_unlock(nq);
	return nqerr_NONE;
}

static int
_nq_queue(pnq_t nq, tstamp_t stamp, char *data, size_t size, unsigned long crc) {
	char fname[NAME_MAX];
	char b[50];
	int r;
	snprintf(fname, NAME_MAX - 1, "%s.%lu", tstamp_fmt_ordered(b, stamp), crc);
	err_debug(">>> write '%s'", fname);
	if (futl_write(fname, data, size)) {
		err_error("cannot write file %s at %p of size %lu", fname, data, size);
		return nqerr_CANNOT_WRITE; 
	}
	
	err_debug(">>> create new entry for '%s'", fname);
	if ((r = _nqe_new(nq, stamp, crc, fname)) != nqerr_NONE) {
		
		futl_rm(fname);
		return r; 
	}
	nq_dump(nq);
	return nqerr_NONE;
}

int
nq_put(pnq_t nq, tstamp_t stamp, char *data, size_t size, unsigned long crc) {
	char b[50];
	err_log("Queuing message %s.%lu", tstamp_fmt_ordered(b, stamp), crc);
	return _nq_queue(nq, stamp, data, size, crc);
}


int
nq_requeue(pnq_t nq, tstamp_t stamp, char *data, size_t size, unsigned long crc) {
	char b[50];
/* TODO: keep track of requeue count */
	err_log("Requeuing message %s.%lu", tstamp_fmt_ordered(b, stamp), crc);
	return _nq_queue(nq, stamp, data, size, crc);
}

int
nq_get(pnq_t nq, char **data, size_t *size, unsigned long *crc) {
	_nq_lock(nq);
	*data = NULL;
	
	err_debug("in nq_get");
//nq_dump(nq);

	if (!nq) {
		err_error(nq_err_string(nqerr_BAD_QUEUE));
		return nqerr_BAD_QUEUE;
	}

	if (!nq->nfiles) {

		err_info(nq_err_string(nqerr_EMPTY));
		return nqerr_EMPTY;
	}
	
	err_debug("loading file %s", nq->e->file);
	*data = futl_load(nq->e->file, size);
	
	if (*size != nq->e->size) {
		err_warning("Attention, wrong size found (having %lu instead of %lu)", *size, nq->e->size);
	}
	*crc = crc_32((unsigned char *) *data, *size);
	if (*crc != nq->e->crc) {
		err_warning("Attention, wrong crc calculated (having %lu instead of %lu)", *crc, nq->e->crc);
	}

	if (nq->nfiles == 1) {
		nq->e = pfree(nq->e);
	} else {
		memmove(nq->e, nq->e + 1, nq->nfiles - 1);
		nq->nfiles--;
		nq->nget++;
		char *p;
		if (!(p = prealloc(nq->e, nq->nfiles * nqe_sizeof()))) {
			err_warning("problem occured while trying to reallocate persistant memory for queue entries");
		} else {
			nq->e = (pnqe_t) p;
		}
	}
	_nq_unlock(nq);
	return nqerr_NONE;
}

int
nq_count(pnq_t nq) {
	return 0;
}

tstamp_t
nq_oldest(pnq_t nq) {
	if (!nq || !nq->e) return tstamp_zero();
	return nq->e->arrival;
}

#if 0
void
nq_dump_content() {
}
#endif
