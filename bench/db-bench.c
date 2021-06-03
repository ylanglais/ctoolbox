#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tbx/tstmr.h>
#include <tbx/db/db.h>
#include <tbx/option.h>
#include <tbx/err.h>
#include "bench.h"

typedef char string_t[64];
typedef char statement_t[4096];

typedef struct {
	long		bench_wait;
	long		bench_cycles;
	long    	db_loops;
	dbtype_t    dbtype;
	string_t	server;
	int			port;
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

int db_bench(void *null) {
	string_t b, b1, b2, b3, b4;	
	pquery_t q;

	for (_c_.niter = 0; _c_.niter < _c_.db_loops; _c_.niter++) { e
		tmr_start(_c_.tmrcnct);
		if (db_new(_c_.dbtype, _c_.server, _c_.port, _c_.dbname, _c_.user, _c_.passwd)) {
			tmr_pause(_c_.tmrcnct);
			err_error("cannot connect to db (type: %s, server: %s, port: %d, base: %s; user: %s)", db_type_to_name);
			return 1;
		}
		tmr_pause(_c_.tmrcnct);
		tmr_start(_c_.tmrexec);
		if (!(q = db_query_new(_c_.statement))) {
			err_error("cannot create a new query");
			db_destroy();
			return 2;
		}
		while (db_next_row());
		tmr_pause(_c_.tmrexec);
		q = db_query_destroy();
		db_destroy();
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
		opt_entry_add(opt, "db_loops",      "%d", sizeof(long),		   &_c_.bb_loops,      NULL);
		opt_entry_add(opt, "DBSERVER", 		"%s", sizeof(string_t),    &_c_.server, 	   NULL);
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
	if (!_c_.db_loops)	    _c_.db_loops      = 10000;
	if (!_c_.server[0]) {
		if (getenv("DBSERVER")) 
		   strcmp(_c_.server, getenv("DBSERVER"));
		else exit(1);
	}
	if (!_c_.dbname[0]) {
		if (getenv("DBNAME")) {
			strcmp(_c_.dbname, getenv("DBNAME"));
		}
		else exit(2);
	}
	if (!_c_.user[0]) {
		if (getenv("DBUSER")) {
			strcmp(_c_.user, getenv("DBUSER"));
		}
		else exit(3);
	}
	if (!_c_.passwd[0]) {
		if (getenv("DBPASS")) {
			strcmp(_c_.user, getenv("DBPASS"));
		}
		else exit(4);
	}

	if (!_c_.statement[0]) {
		err_error("no test query provided");
		exit(1);
	}

	if (!(_c_.tmrcnct = tmr_new())) fprintf(stderr, "cannot create connection timer\n");
	if (!(_c_.tmrexec = tmr_new())) fprintf(stderr, "cannot create execution timer\n");

	bench_prepare(db_bench, NULL, _c_.bench_wait, _c_.bench_cycles, _c_.outfile);
	bench_run(NULL);
	bench_statistics();

	tmr_destroy(_c_.tmrcnct);
	tmr_destroy(_c_.tmrexec);

	return 0;
}
