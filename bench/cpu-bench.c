
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tbx/tstmr.h>
#include <tbx/hash.h>
#include <tbx/option.h>

#include "bench.h"

#include "colors.def"

typedef struct {
	long	bench_wait;
	long	bench_cycles;
	long 	hashing_loops;
} conf_t, *pconf_t;

static conf_t _c_;

int
hashing() {
	int i, j, n;
	phash_t h;

	if (!(h = hash_new())) return 1;
	
	for (n = 0; colors[n].name[0] != 0; n++) {
		//printf("colors[%d].name = %s\n", n, colors[n].name);
		hash_insert(h, colors[n].name, (void *) (colors + n));	
	}
	
	for (j = 0; j < _c_.hashing_loops; j++) for (i = 0; i < n; i++) hash_retrieve(h, colors[i].name);
 
	h = hash_destroy(h);
	return 0;	
}

int cpu(void *null) {
	char b[60];
	ptmr_t timer;
	timer = tmr_new();

	/* part 1 : hashing tests */
	tmr_start(timer);
	hashing();
	tmr_pause(timer);
	printf(">>> CPU test (hashing) : %s\n", tstamp_duration_fmt(b, tmr_elapsed(timer)));
	tmr_reset(timer);
	
	/* part 2 :  arthimetics  */
	/* 
	tmr_start(timer);
	double_arthm();
	tmr_pause(timer);
	*/

	tmr_destroy(timer);
	return 0;
}

int main(int n, char *a[]) {
	popt_t opt;

	memset(&_c_, 0, sizeof(conf_t));

	if (n >= 2) {
		opt = opt_new("[ \t]*([A-z0-9_]+)[ \t]*=[ \t]*(.*)[ \t]*\n");
		opt_entry_add(opt, "bench_wait",    "%d", sizeof(long), &_c_.bench_wait,    NULL);
		opt_entry_add(opt, "bench_cycles",  "%d", sizeof(long), &_c_.bench_cycles,  NULL);
		opt_entry_add(opt, "hashing_loops", "%d", sizeof(long), &_c_.hashing_loops, NULL);
		opt_parse(opt, a[1], &_c_);
		opt_destroy(opt);
	} 
	
	if (!_c_.bench_wait)    _c_.bench_wait    =     5;
	if (!_c_.bench_cycles)	_c_.bench_cycles  =    10;
	if (!_c_.hashing_loops)	_c_.hashing_loops = 10000;

	bench_prepare(cpu, NULL, _c_.bench_wait, _c_.bench_cycles, NULL);
	bench_statistics();
	bench_run(NULL);
	bench_statistics();
	return 0;
}
