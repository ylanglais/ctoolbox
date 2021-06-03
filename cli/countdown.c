#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <tbx/rexp.h>
#include <tbx/tstmr.h>

tstamp_t
ts_parse(char *str) {
	char *s;
	tstamp_t ts;
	prexp_t re;
	struct tm tm = {0, 0, 0, 0, 0, 0, 0, 0, 0};
				/* 12            3            4              567              89               A           B   C*/
	char spec[] = "(([0-9]{0,4})/([0-9]{0,2})/([0-9]{0,2} ))?((([0-9]{0,2}):)?(([0-9]{0,2}):))?([0-9]{0,2})(\\.([0-9]{3}))?";
	time_t t;

	ts = tstamp_zero();

	if (!(re = rexp_new(str, spec, rexp_EXTENDED | rexp_NL_DO_NOT_MATCH))) {
		fprintf(stderr, "bad string\n");
		return ts;
	}

	if (rexp_find(re)) {
		if ((s = rexp_sub_get(re, 2))) {
			printf("year :  %s\n", s);
			tm.tm_year = atoi(s);

			free(s);
		} 
		if ((s = rexp_sub_get(re, 3))) {
			printf("month : %s\n", s);
			tm.tm_mon = atoi(s);
			free(s);
		}
		if ((s = rexp_sub_get(re, 4))) {
			printf("day   : %s\n", s);
			tm.tm_mday = atoi(s);
			free(s);
		}
		if ((s = rexp_sub_get(re, 7))) {
			printf("hour  : %s\n", s);
			tm.tm_hour = atoi(s);
			free(s);
		}
		if ((s = rexp_sub_get(re, 9))) {
			printf("min   : %s\n", s);
			tm.tm_min = atoi(s);
			free(s);
		}
		if ((s = rexp_sub_get(re, 10))) {
			printf("sec   : %s\n", s);
			tm.tm_sec = atoi(s);
			free(s);
		}
		if ((s = rexp_sub_get(re, 12))) {
			printf("cent  : %s\n", s);
			ts.tv_usec = atoi(s) * 1000;
			free(s);
		}
	}

	if (tm.tm_year < 1900) {
		ts.tv_sec  = tm.tm_year * 3600 * 24 * 365;
		ts.tv_sec += tm.tm_mon  * 3600 * 24 * 30;
		ts.tv_sec += tm.tm_mday * 3600 * 24;
		ts.tv_sec += tm.tm_hour * 3600; 
		ts.tv_sec += tm.tm_min  * 60;
		ts.tv_sec += tm.tm_sec;
	} else {
		tm.tm_year -= 1900;
		tm.tm_mon  -= 1;
		t = mktime(&tm);
		ts.tv_sec = t;
	}
	
	rexp_destroy(re);
	return ts;	
}


int main(int n, char *a[]) {
	char b[64];
	tstamp_t ts, now; 

	if (n < 2) { 
		printf("nothing to countdown\n");	
		return 1;
	}

/*
	ts = ts_parse(a[1]);
	zero = ones = tstamp_zero();

	ones.tv_sec = 1;

	printf("%s", tstamp_duration_fmt(b, ts)); fflush(stdout);
	while (tstamp_cmp(zero, ts) > 0) {
		sleep(1); 
		printf("\b\b\b\b\b\b\b\b\b\b\b\b%s", tstamp_duration_fmt(b, ts = tstamp_sub(ts, ones))); fflush(stdout);
	}
	printf("\n");
*/

	ts  = ts_parse(a[1]);
	now = tstamp_add(ts, tstamp_get());

	printf("%s\b ", tstamp_duration_fmt(b, ts)); fflush(stdout);

	while (tstamp_cmp(ts = tstamp_get(), now) > 0) {
		usleep(100);
		printf("\b\b\b\b\b\b\b\b\b\b\b\b%s\b ", tstamp_duration_fmt(b, tstamp_sub(now, ts))); fflush(stdout);

	}

	printf("\n");
	

/* 

	now = tstamp_add(ts, tstamp_get());

	while (tstamp_cmp(tstamp_get(), now) > 0) {
		sleep(1);
		printf("%s\n", tstamp_duration_fmt(b, tstamp_sub(now, tstamp_get())));
	}
*/

	


	return 0;
}




/* 
#define echo_cmd(x) { printf(">>> perform command: %s\n", #x); x; }
int main() {
	char b[64], b1[64], b2[64];
	tstamp_t ts, now;

	echo_cmd(ts = ts_parse("1970/01/01 12:13:14.123"));
	printf("ts = %s\n\n", tstamp_fmt(b, ts));

	echo_cmd(ts = ts_parse("2009/03/07 12:13:14.123"));
	printf("ts = %s\n", tstamp_duration_fmt(b, ts));
	printf("ts = %s\n\n", tstamp_fmt(b, ts));

	echo_cmd(ts = ts_parse("0/0/1 12:13:14"));
	printf("ts = %s\n\n", tstamp_duration_fmt(b, ts));
	echo_cmd(ts = ts_parse("12:13:14"));
	printf("ts = %s\n\n", tstamp_duration_fmt(b, ts));
	echo_cmd(ts = ts_parse("13:14"));
	printf("ts = %s\n\n", tstamp_duration_fmt(b, ts));
	echo_cmd(ts = ts_parse("13:14.1234"));
	printf("ts = %s\n\n", tstamp_duration_fmt(b, ts));
	echo_cmd(ts = ts_parse("14.1234"));
	printf("ts = %s\n\n", tstamp_duration_fmt(b, ts));
	echo_cmd(ts = ts_parse("14"));
	printf("ts = %s\n\n", tstamp_duration_fmt(b, ts));

	echo_cmd(ts = ts_parse("1970/05/21 13:40:00"));
	printf("ts = %s\n", tstamp_duration_fmt(b, ts));
	printf("ts = %s\n\n", tstamp_fmt(b, ts));
	
	now = tstamp_get();

	printf("time from %s and %s = %s\n", tstamp_fmt(b1, now), tstamp_fmt(b2, ts), tstamp_duration_fmt(b, tstamp_sub(now, ts)));

	return 0;
}

*/			
	
