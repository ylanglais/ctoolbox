#ifndef _nq_h_
#define _nq_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <tbx/tstmr.h>
#include "qe.h"

#ifndef _nq_c_
typedef enum {
	nqerr_NONE,
	nqerr_EMPTY,
	nqerr_NO_PERSISTANT_MEMORY,
	nqerr_BAD_QUEUE,
	nqerr_BAD_QUEUE_ENTRY,
	nqerr_DIRTY_QUEUE,
	nqerr_CANNOT_WRITE
} nqerr_t;

typedef enum {
	nq_NONE,
	nq_NEW,
	nq_INITIALIZED,
	nq_CHECKING,
	nq_DIRTY,
	nq_CLEANING,
	nq_RUNNING,
	nq_LOCKED,
	nq_UNLOCK,
	nq_PAUSED,
	nq_STOPPED
} nq_state_t;

typedef void * pnq_t;
typedef void * pnqe_t;
#endif
pnq_t	   nq_load();
void       nq_dump(pnq_t nq);
char * 	   nq_err_string(nqerr_t err);
pnq_t      nq_new(char *path);
pnq_t      nq_destroy(pnq_t nq);
nq_state_t nq_state(pnq_t nq);

int        nq_put(pnq_t nq, pqe_t qe);
int        nq_requeue(pnq_t nq, pqe_t qe);
int        nq_get(pnq_t nq, pqe_t *qe);
int		   nq_got(pnq_t nq, pqe_t qe);

int        nq_count(pnq_t nq);

#ifdef __cplusplus
}
#endif

#endif
