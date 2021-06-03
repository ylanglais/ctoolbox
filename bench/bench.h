
#ifndef _bench_h_
#define _bench_h_

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*worker_f)(void *);

int  bench_prepare(worker_f worker, void *param, long wait, long max_iter, char *filename);
int  bench_run(void *param);
void bench_statistics();

#ifdef __cplusplus
}
#endif

#endif

