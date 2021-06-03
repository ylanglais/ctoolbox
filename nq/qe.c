#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <linux/limits.h>

#include <tbx/tstmr.h>
#include <tbx/err.h>
#include <tbx/futl.h>

#define qe_MAX_ADDR_LEN   16
#define qe_MAX_FNAME_LEN 256

typedef struct {
	tstamp_t arrival;
	size_t	 crc;
	size_t	 size;
	char 	 from[qe_MAX_ADDR_LEN];
	char     file[qe_MAX_FNAME_LEN]; 
	int		 requeued;
} qe_t, *pqe_t;

#define  _qe_c_
#include "qe.h"
#undef  _qe_c_

size_t 
qe_sizeof() { 
	return sizeof(qe_t); 
}

/* 
pqe_t
qe_from_file(char *file, tstamp_t arrival, size_t crc, char *from, int requeued) {
	pqe_t qe;
	if (!(qe = (pqe_t) malloc(sizeof(qe_t)))) {
		return NULL;
	}
	qe->arrival  = arrival;
	qe->crc      = crc;
	qe->requeued = requeued;
	strncpy(qe->from, from, qe_MAX_ADDR_LEN);
	strncpy(qe->file, file, qe_MAX_FNAME_LEN);
	qe_save(qe);
	return qe;
}
*/

pqe_t
qe_new(char *data, size_t size, tstamp_t arrival, size_t crc, char *from) {
	pqe_t qe;
	char fname[NAME_MAX + 1];
	char b[30];

	snprintf(fname, NAME_MAX, "%s.%lu", tstamp_fmt_ordered(b, arrival), crc);

	futl_write(fname, data, size);

	if (!(qe = (pqe_t) malloc(sizeof(qe_t)))) {
		return NULL;
	}
	qe->arrival  = arrival;
	qe->crc      = crc;
	qe->requeued = 0;
	strncpy(qe->from, from, qe_MAX_ADDR_LEN);
	strncpy(qe->file, fname, qe_MAX_FNAME_LEN);
	qe_save(qe);

	return qe;
}

pqe_t 
qe_destroy(pqe_t qe) {
	if (qe) {
		free(qe);
	}
	return NULL;
}

pqe_t
qe_load(char *filename) {
	char fname[NAME_MAX+1];
	size_t size;
	pqe_t qe;

	sprintf(fname, "*%s.hdr", filename);
	qe = (pqe_t) futl_load(fname, &size);
	return qe;
}

int 
qe_save(pqe_t qe) {
	char fname[NAME_MAX+1];
	if (!qe) return 1;

	sprintf(fname, "%s.hdr", qe->file);
	return futl_write(fname, (char *) qe, sizeof(qe_t));
}

int 
qe_clear(pqe_t qe) {
	char fname[NAME_MAX+1];

	if (!qe) return 1;

	sprintf(fname, "%s.hdr", qe->file);
	futl_rm(fname);
	return 0;		
}

void
qe_dump(pqe_t qe) {
	char b[100];
	if (!qe) {
		printf("\tNull queue entry");
		return;
	}
	printf("\tfrom:	   %s",  qe->from);
	printf("\tarrival: %s",  tstamp_fmt(b, qe->arrival));
	printf("\tfile:	   %s",  qe->file);
	printf("\tsize:    %lu", qe->size);
	printf("\tcrc:     %lu", qe->crc);
}

int
qe_requeued(pqe_t qe) {
	if (!qe) return 0;
	return qe->requeued;	
}

int	
qe_requeue_incr(pqe_t qe) {
	if (!qe) return 0;
	return ++qe->requeued;
}

char *
qe_data(pqe_t qe, size_t *size) {
	if (!qe) return NULL;
	return futl_load(qe->file, size);
}

tstamp_t
qe_arrival(pqe_t qe) {
	if (!qe) return tstamp_zero();
	return qe->arrival;
}

unsigned long 
qe_crc(pqe_t qe) {
	if (!qe) return 0;
	return qe->crc;
}
char *
qe_file(pqe_t qe) {
	if (!qe) return NULL;
	return qe->file;
}

size_t
qe_size(pqe_t qe) {
	if (!qe) return 0;
	return qe->size;
}

char *
qe_from(pqe_t qe) {
	if (!qe) return NULL;
	return qe->from;
}

