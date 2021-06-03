
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <sys/times.h>
#include <sys/wait.h>

#ifndef CLK_TCK
#define CLK_TCK sysconf(_SC_CLK_TCK)
#endif

#include <tbx/tstmr.h>
#include "bench.h"

typedef enum {
	benchNONE,
	benchREADY,
	benchRUNNING,
	benchWAITING,
	benchSTOPPED
} bench_status_t;

static char *_bench_status_strings[] = {
	"NONE",
	"READY",
	"RUNNNING",
	"WAITING",
	"STOPPED"
};

typedef struct {
	bench_status_t status;	
	long 		max_iter;
	void *		param;
	worker_f	worker;
	long		wait;
	long		cyclenum;
	tstamp_t 	global_cycle_duration;
	tstamp_t 	cycle_avg_duration;
	struct tms	current;
	struct tms  prev;
	tstamp_t    global_sys_duration;
	tstamp_t    global_usr_duration;
} bench_t, *pbench_t;

FILE *_bench_output_ = NULL;

static bench_t _bench_;

int
bench_prepare(worker_f worker, void *param, long wait, long max_iter, char *filename) {
	if (!worker) return 1;
	_bench_.worker   = worker;
	_bench_.param    = param;
	_bench_.wait     = wait;
	_bench_.max_iter = max_iter;
	_bench_.status   = benchREADY;
	_bench_.cyclenum = 0;
	_bench_.global_cycle_duration = _bench_.cycle_avg_duration  = 
	_bench_.global_sys_duration   = _bench_.global_usr_duration = tstamp_zero();

	_bench_.current.tms_stime = _bench_.current.tms_utime = _bench_.current.tms_cstime = _bench_.current.tms_cutime = 
	_bench_.prev.tms_stime    = _bench_.prev.tms_utime    = _bench_.prev.tms_cstime    = _bench_.prev.tms_cutime    = 0;

	if (filename) _bench_output_ = fopen(filename, "w");
	if (!_bench_output_) _bench_output_ = stdout;

	return 0;
}

int
bench_run(void *param) {
	tstamp_t last_sys_duration, last_usr_duration;
	ptmr_t tmr;
	char b[100], b1[100], b2[100], b3[100], b4[100], b5[100], b6[100];
	pid_t pid;
	int lock;
	int code;
	double elapsed;

	if (!param) param = _bench_.param;
	
	if (!(tmr = tmr_new())) {
		fprintf(stderr, "Cannot create timer !!!");
		return(2);
	}

	while (!_bench_.max_iter || _bench_.cyclenum < _bench_.max_iter) {
		_bench_.status = benchRUNNING;

		_bench_.prev = _bench_.current;

		tmr_start(tmr);	

		if (!(pid = fork())) {
			if ((code = _bench_.worker(param))) {
				fprintf(stderr, "worker returned = %d\n", code);
			}
			exit(code);
		} else {
			waitpid(pid, &lock, 0);
		}
		times(&_bench_.current);

		tmr_pause(tmr);

		_bench_.cyclenum++;
		_bench_.global_cycle_duration = tstamp_add(_bench_.global_cycle_duration, tmr_elapsed(tmr));
		_bench_.cycle_avg_duration    = tstamp_div(_bench_.global_cycle_duration, _bench_.cyclenum);


		elapsed = (double) (_bench_.current.tms_cstime - _bench_.prev.tms_cstime) / (double) CLK_TCK;
		last_sys_duration.tv_sec  = elapsed;
		last_sys_duration.tv_usec = (elapsed - (double) last_sys_duration.tv_sec) * 1000000.;
		
		elapsed = (double) (_bench_.current.tms_cutime - _bench_.prev.tms_cutime) / (double) CLK_TCK;
		last_usr_duration.tv_sec  = elapsed;
		last_usr_duration.tv_usec = (elapsed - (double) last_usr_duration.tv_sec) * 1000000.;

		_bench_.global_sys_duration = tstamp_add(_bench_.global_sys_duration, last_sys_duration);
		_bench_.global_usr_duration = tstamp_add(_bench_.global_usr_duration, last_usr_duration);

		fprintf(_bench_output_, "%s cycle %d completed in elapesed %s sys %s user %s (avg : elapsed  %s sys %s user %s)\n", 
			tstamp_fmt(b, tstamp_get()),
			_bench_.cyclenum, 
			tstamp_duration_fmt(b1, tmr_elapsed(tmr)), 
			tstamp_duration_fmt(b2, last_sys_duration),
			tstamp_duration_fmt(b3, last_usr_duration),
			tstamp_duration_fmt(b4, _bench_.cycle_avg_duration),
			tstamp_duration_fmt(b5, tstamp_div(_bench_.global_sys_duration, _bench_.cyclenum)),
			tstamp_duration_fmt(b6, tstamp_div(_bench_.global_usr_duration, _bench_.cyclenum)));
		_bench_.status = benchWAITING;
		tmr_reset(tmr);
		if (_bench_.max_iter && _bench_.cyclenum < _bench_.max_iter) sleep(_bench_.wait);
	}
	_bench_.status = benchSTOPPED;
	return 0;
}

void 
bench_statistics() {
	char b1[100], b2[100];

	if (_bench_.status >= benchNONE && _bench_.status <= benchSTOPPED) 
		fprintf(_bench_output_, "Current bench status : %s\n", _bench_status_strings[_bench_.status]);
	else
		fprintf(_bench_output_, "Current bench status undetermined\n");

	fprintf(_bench_output_, "cycles completed : %d\n", _bench_.cyclenum);
	fprintf(_bench_output_, "global cycle duration : %s\n", tstamp_duration_fmt(b1, _bench_.global_cycle_duration));
	fprintf(_bench_output_, "global sys   duration : %s\n", tstamp_duration_fmt(b1, _bench_.global_sys_duration));
	fprintf(_bench_output_, "global user  duration : %s\n", tstamp_duration_fmt(b1, _bench_.global_usr_duration));

	if (_bench_.cyclenum) { 
		fprintf(_bench_output_, "average cycle duration : %s\n", tstamp_duration_fmt(b2, tstamp_div(_bench_.global_cycle_duration, _bench_.cyclenum)));
		fprintf(_bench_output_, "average sys duration : %s\n", tstamp_duration_fmt(b2, tstamp_div(_bench_.global_sys_duration, _bench_.cyclenum)));
		fprintf(_bench_output_, "average usr duration : %s\n", tstamp_duration_fmt(b2, tstamp_div(_bench_.global_usr_duration, _bench_.cyclenum)));
	}
}

#ifdef _test_bench_

int work(void *param) {
	int i;
	int j;
	
	for (i = 0; i < 10000; i++) for (j = 0; j < 20000; j++);
	return 0;
}

int main(void) {
	bench_prepare(work, NULL, 0, 10, NULL);
	bench_statistics();
	bench_run(NULL);
	bench_statistics();
	return 0;
}
#endif

