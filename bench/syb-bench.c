#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tbx/bench/bench.h>
#include <tbx/tmr.h>
#include <tbx/dsybase.h>
#include <option.h>
#include <err.h>

typedef char string_t[64];
typedef char statement_t[4096];

typedef struct {
	long	bench_wait;
	long	bench_cycles;
	long    syb_loops;
	string_t	dsquery;
	string_t    dbname;
	string_t 	user;
	string_t 	passwd;
	statement_t statement;
	statement_t outfile;
	ptmr_t		tmrcnct;
	ptmr_t		tmrexec;
	tstamp_t	durtotcnct;	
	tstamp_t    durtotexec;
	long 		niter;
} conf_t, *pconf_t;

static conf_t _c_;
extern FILE *_bench_output_;

int syb(void *null) {
	psyb_t s;
	string_t b, b1, b2, b3, b4;	

	for (_c_.niter = 0; _c_.niter < _c_.syb_loops; _c_.niter++) { e
		tmr_start(_c_.tmrcnct);
		if (!(s = syb_new(_c_.dsquery, _c_.dbname, _c_.user, _c_.passwd))) {
			tmr_pause(_c_.tmrcnct);
			fprintf(stderr, "cannot connect to sybase\n");
			return 1;
		}
		tmr_pause(_c_.tmrcnct);

		tmr_start(_c_.tmrexec);
		syb_cmd(s, _c_.statement);
		while (syb_next_row(s));
		tmr_pause(_c_.tmrexec);
		s = syb_destroy(s);
	}

	_c_.durtotcnct = tstamp_add(_c_.durtotcnct, tmr_elapsed(_c_.tmrcnct));
	_c_.durtotexec = tstamp_add(_c_.durtotexec, tmr_elapsed(_c_.tmrexec));

//	fprintf(">>> connect: %d.%d exec: %d.%d\n", _c_.durtotcnct.tv_sec, _c_.durtotcnct.tv_usec, _c_.durtotexec.tv_sec, _c_.durtotexec.tv_usec);
	fprintf(stdout, "%s SYBASE LOOP : cnx avg %s, tot %s; exec avg %s, tot %s on %ld iterations\n",
		tstamp_fmt(b, tstamp_get()),
		tstamp_duration_fmt(b1, tstamp_div(_c_.durtotcnct, _c_.niter)),
		tstamp_duration_fmt(b2, _c_.durtotcnct),
		tstamp_duration_fmt(b3, tstamp_div(_c_.durtotexec, _c_.niter)),
		tstamp_duration_fmt(b4, _c_.durtotexec),
		_c_.niter);
	
	return 0;
}

int main(int n, char *a[]) {
	popt_t opt;

	memset(&_c_, 0, sizeof(conf_t));

	if (n >= 2) {
		opt = opt_new("[ \t]*([A-z0-9_]+)[ \t]*=[ \t]*(.*)[ \t]*\n");
		opt_entry_add(opt, "bench_wait",    "%d", sizeof(long), 	   &_c_.bench_wait,    NULL);
		opt_entry_add(opt, "bench_cycles",  "%d", sizeof(long), 	   &_c_.bench_cycles,  NULL);
		opt_entry_add(opt, "syb_loops",     "%d", sizeof(long),		   &_c_.syb_loops,     NULL);
		opt_entry_add(opt, "DSQUERY", 		"%s", sizeof(string_t),    &_c_.dsquery, 	   NULL);
		opt_entry_add(opt, "DBNAME", 		"%s", sizeof(string_t),    &_c_.dbname, 	   NULL);
		opt_entry_add(opt, "USER", 			"%s", sizeof(string_t),    &_c_.user, 	  	   NULL);
		opt_entry_add(opt, "PASSWD", 		"%s", sizeof(string_t),    &_c_.passwd,        NULL);
		opt_entry_add(opt, "STATEMENT", 	"%s", sizeof(statement_t), &_c_.statement,     NULL);
		opt_entry_add(opt, "outfile", 		"%s", sizeof(statement_t), &_c_.outfile,       NULL);
		opt_parse(opt, a[1], &_c_);
		opt_destroy(opt);
	} 
	
	if (!_c_.bench_wait)    _c_.bench_wait    =     5;
	if (!_c_.bench_cycles)	_c_.bench_cycles  =    10;
	if (!_c_.syb_loops)	    _c_.syb_loops     = 10000;
	if (!_c_.dsquery[0]) {
		if (getenv("DSQUERY")) 
		   strcmp(_c_.dsquery, getenv("DSQUERY"));
		else exit(1);
	}
	if (!_c_.dbname[0]) {
		if (getenv("SUMMITDBNAME")) {
			strcmp(_c_.dbname, getenv("SUMMITDBNAME"));
		}
		else exit(2);
	}
	if (!_c_.user[0]) {
		if (getenv("SUM_USER")) {
			strcmp(_c_.user, getenv("SUM_USER"));
		}
		else exit(3);
	}
	if (!_c_.passwd[0]) {
		if (getenv("SUM_PASS")) {
			strcmp(_c_.user, getenv("SUM_PASS"));
		}
		else exit(4);
	}

	if (!_c_.statement[0]) strcmp(_c_.statement, "select Today from dmLOCATION where Audit_Current = 'Y' and LocationName = 'CDCMPARIS'");

	if (!(_c_.tmrcnct = tmr_new())) fprintf(stderr, "cannot create connection timer\n");
	if (!(_c_.tmrexec = tmr_new())) fprintf(stderr, "cannot create execution timer\n");

	bench_prepare(syb, NULL, _c_.bench_wait, _c_.bench_cycles, _c_.outfile);
	bench_run(NULL);
	bench_statistics();

	//printf("sybase specific statistics:\n");
	
	tmr_destroy(_c_.tmrcnct);
	tmr_destroy(_c_.tmrexec);

	return 0;
}
