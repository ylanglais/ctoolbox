#ifndef _frq_h_
#define _frq_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "ampl.h"

#ifndef _frq_c_
typedef void *pfrq_t;
#endif

int      frq_nfreqs();
double   frq_frequency(int i);

pfrq_t   frq_new(pampl_t a, void (*progress_cb)(void *, double), void *data);
pfrq_t   frq_win_new(pampl_t a, double twin, void (*progress_cb)(void *, double), void *data);
pfrq_t   frq_new_thpo(pampl_t a, int nthreads, void (*progress_cb)(void *), void *data);
pfrq_t   frq_destroy(pfrq_t  frq);

double   frq_get(pfrq_t frq, int i, int j);
double   frq_at(pfrq_t frq,  double time, int j);
int		 frq_samples(pfrq_t  frq);
double   frq_tunit(pfrq_t frq); 
double   frq_duration(pfrq_t frq); 
double * frq_ptr(pfrq_t frq);

void     frq_dump(pfrq_t frq, char *fname);
void     frq_freq_dump(pfrq_t frq, int i, char *fname);

pfrq_t   frq_load(char *fname);
int      frq_save(pfrq_t frq, char *fname);

#ifdef __cplusplus
}
#endif

#endif

