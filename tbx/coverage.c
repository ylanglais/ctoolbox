#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>

#include <sys/time.h>
#include <unistd.h>

#include "coverage.h"
	
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
		30/05/2016	1.1	Adding changelog

*/

char cov_MODULE[]  = "Code coverage analysis helper";
char cov_PURPOSE[] = "Code coverage analysis helper";
char cov_VERSION[] = "1.1";
char cov_DATEVER[] = "30/05/2016";

static FILE *cov_file = NULL;
static int cov_level = 0;
static struct timeval cov_start = {-6, -6};

char *cov_timestamp() {
	static int count = 0;
	static float last = 0;
	float new;
	char *buff;
	struct timeval tvp;
	struct timezone tz;

	if (cov_start.tv_sec == -6) gettimeofday(&cov_start, &tz);

	gettimeofday(&tvp, &tz);
	buff = (char *) malloc(64 * sizeof(char));
	tvp.tv_sec -= cov_start.tv_sec;
	tvp.tv_usec -= cov_start.tv_usec;
	if (tvp.tv_usec < 0) {
		tvp.tv_usec += 1000000; 
		tvp.tv_sec -= 1;
	}
	new = (float) tvp.tv_sec + (float) tvp.tv_usec/ 1000000.;
	sprintf(buff, "%d %ld.%06ld %f", count, (long) tvp.tv_sec, (long) tvp.tv_usec, new - last);
	last = new;
	++count;
	return buff;	
}

char *cov_time_fmt(struct timeval tv) {
	char *buff;
	buff = (char *) malloc(64 * sizeof(char));
	sprintf(buff, "%ld.%06ld", (long) tv.tv_sec, (long) tv.tv_usec);
	return buff;
}

struct timeval cov_time_zero() {
	struct timeval tvp;
	tvp.tv_sec = tvp.tv_usec = 0;
	return tvp;
}

struct timeval cov_time_get() {
	struct timeval tvp;
	struct timezone tz;
	gettimeofday(&tvp, &tz);
	return tvp;
}

struct timeval cov_time_sub(struct timeval tv0, struct timeval tv1) {
	struct timeval tvd;
	tvd.tv_sec  = tv0.tv_sec  - tv1.tv_sec;
	tvd.tv_usec = tv0.tv_usec - tv1.tv_usec;
	if (tvd.tv_usec < 0) {
		tvd.tv_usec += 1000000; 
		tvd.tv_sec -= 1;
	}
	return tvd;
}

struct timeval cov_time_add(struct timeval tv0, struct timeval tv1) {
	struct timeval tvs;
	tvs.tv_sec  = tv0.tv_sec  + tv1.tv_sec;
	tvs.tv_usec = tv0.tv_usec + tv1.tv_usec;
	if (tvs.tv_usec >= 1000000) {
		tvs.tv_usec -= 1000000; 
		tvs.tv_sec  += 1;
	}
	return tvs;
}

void cov_close() {
	char *tt;
	
	if (!cov_file) return;
	fprintf(cov_file, "%s Ending coverage\n", tt=cov_timestamp());
	if (tt) free(tt);
	fclose(cov_file);
}

void cov_open() {
	char *tt;
	if (cov_file) return;
	cov_file = fopen(".coverage", "w");
	if (cov_file) {
		atexit(cov_close);
		fprintf(cov_file, "%s Starting coverage\n", tt=cov_timestamp());
	if (tt) free(tt);
	}
}

void 
coverage(char *file, int line, char *function, int flag) {
	char *t, tb[32], *tt;
	if (!cov_file) cov_open();
	if (!cov_file) return;
	if (flag == 1) {
		cov_level++; 
		t = "in";
	} else if (!flag) { 
		cov_level--;
		t = "out";
	} else {
		sprintf(tb, "check point %d", flag);
		t = tb;
	}
	fprintf(cov_file, "%s %d %s:%d %s %s\n", tt=cov_timestamp(), cov_level, file, line, function, t);
	if (tt) free(tt);
}
