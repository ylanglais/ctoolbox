#ifndef _qe_h_
#define _qe_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <tbx/tstmr.h>

#ifndef _qe_c_
typedef void *pqe_t;
#endif

size_t        qe_sizeof();
pqe_t         qe_new(char *data, size_t size, tstamp_t arrival, size_t crc, char *from);
//pqe_t         qe_from_file(char *file, tstamp_t arrival, size_t crc, char *from);
pqe_t         qe_destroy(pqe_t qe);

pqe_t         qe_load(char *filename);
int           qe_save(pqe_t qe);
int           qe_clear(pqe_t   qe);

int	   		  qe_requeue_incr(pqe_t qe);

char *        qe_data(pqe_t qe, size_t *size);
unsigned long qe_crc(pqe_t qe);
char *        qe_file(pqe_t qe);
char *		  qe_from(pqe_t qe);
size_t  	  qe_size(pqe_t qe);
tstamp_t  	  qe_arrival(pqe_t qe);
void          qe_dump(pqe_t qe);

#ifdef __cplusplus
}
#endif

#endif
