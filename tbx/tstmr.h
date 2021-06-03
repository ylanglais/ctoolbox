#ifndef _tstmr_h_
#define _tstmr_h_
#include <time.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _tstmr_c_
typedef enum {
	tmrUNKOWN,
	tmrZERO,
	tmrRUNNING,
	tmrPAUSED,
	tmrSTOPPED,
	tmrERROR,
	tmrBADTIMER
} tmr_state_t;

typedef struct timeval tstamp_t, *ptstamp_t;
typedef struct _tmr_t *ptmr_t;
#endif

/* timestamp part: */
tstamp_t  tstamp_set(time_t t);
tstamp_t  tstamp_add(tstamp_t ts1, tstamp_t ts2);
tstamp_t  tstamp_sub(tstamp_t ts2, tstamp_t ts1);
tstamp_t  tstamp_mul(tstamp_t ts1, int k);
tstamp_t  tstamp_div(tstamp_t ts1, int k);
tstamp_t  tstamp_zero();
tstamp_t  tstamp_get();
int       tstamp_cmp(tstamp_t ts1, tstamp_t ts2);
char     *tstamp_fmt(char *buff, tstamp_t ts);
char     *tstamp_fmt_fr(char *buff, tstamp_t ts);
char     *tstamp_fmt_ordered(char *buff, tstamp_t ts);
char     *tstamp_fmt_db(char *buff, tstamp_t ts);
char     *tstamp_duration_fmt(char *buff, tstamp_t ts);

/* timer part: */
ptmr_t    tmr_new();
ptmr_t    tmr_destroy(ptmr_t t);
tstamp_t  tmr_elapsed(ptmr_t t);
int       tmr_reset(ptmr_t t);
int       tmr_start(ptmr_t t);
int       tmr_stop(ptmr_t t);
tstamp_t  tmr_pause(ptmr_t t);
tstamp_t  tmr_laps(ptmr_t t);
void      tmr_dump(ptmr_t t);

#ifdef __cplusplus
}
#endif

#endif
