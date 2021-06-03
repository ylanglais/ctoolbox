#ifndef _thpo_h_
#define _thpo_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _thpo_c_
typedef void *pthpo_t;
#endif

enum {
	thpo_NONE,
	thpo_CREATED,
	thpo_READY,
	thpo_STARTED,
	thpo_RUNNING,
	thpo_PROCESSED,
	thpo_INVALID
};

enum {
	thpo_DATA_WAIT,
	thpo_DATA_DONE,
	thpo_DATA_STOP,
	thpo_DATA_INVALID
};

pthpo_t thpo_new(int nthreads, int (*worker_cb)(void *));
pthpo_t thpo_destroy(pthpo_t tp);
int     thpo_push(pthpo_t tp, unsigned long dataid, void *data);
int     thpo_run_blocking(pthpo_t tp);
int     thpo_run(pthpo_t tp);
int		thpo_status(pthpo_t tp);
int		thpo_data_status(pthpo_t tp);
int     thpo_done(pthpo_t tp);
int		thpo_data_done(pthpo_t tp);
int     thpo_data_stop(pthpo_t tp);
void	thpo_stats_dump(pthpo_t tp);
void	thpo_stats_detail(pthpo_t tp);

#ifdef __cplusplus
}
#endif

#endif 
