#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	30/05/2016  2.1  add tbx std tags
	04/08/2016  2.2  add changelog
*/   

char tmr_MODULE[]  = "Timer";
char tmr_PURPOSE[] = "Timer";
char tmr_VERSION[] = "2.2";
char tmr_DATEVER[] = "04/08/2016";

typedef struct timeval tstamp_t, *ptstamp_t;

typedef enum {
	tmrUNKOWN,
	tmrZERO,
	tmrRUNNING,
	tmrPAUSED,
	tmrSTOPPED,
	tmrERROR,
	tmrBADTIMER
} tmr_state_t;
	
typedef struct {
	tmr_state_t	state;	
	tstamp_t 	start;
	tstamp_t 	elapsed;
} tmr_t, *ptmr_t;

#define _tstmr_c_
#include "tstmr.h"
#undef  _tstmr_c_

/* timestamp part: */
tstamp_t
tstamp_add(tstamp_t ts1, tstamp_t ts2) {
	tstamp_t ts;

	ts.tv_sec  = ts1.tv_sec  + ts2.tv_sec;
	ts.tv_usec = ts1.tv_usec + ts2.tv_usec;
	if (ts.tv_usec >= 1000000) {
		ts.tv_usec -= 1000000; 
		ts.tv_sec  += 1;
	}
	return ts;
}

tstamp_t
tstamp_sub(tstamp_t ts1, tstamp_t ts2) {
	tstamp_t ts;
	ts.tv_sec  = ts1.tv_sec  - ts2.tv_sec;
	ts.tv_usec = ts1.tv_usec - ts2.tv_usec;
	if (ts.tv_usec < 0) {
		ts.tv_usec += 1000000; 
		ts.tv_sec -= 1;
	}
	return ts;
}

tstamp_t
tstamp_mul(tstamp_t ts1, int k) {
	tstamp_t ts;
	ts.tv_sec  = k * ts1.tv_sec;
	ts.tv_usec = k * ts1.tv_usec;

	if (ts.tv_usec >= 1000000) {
		ts.tv_sec += ts.tv_usec / 1000000;
		ts.tv_usec = ts.tv_usec % 1000000;
	}

	return ts;
}

tstamp_t
tstamp_div(tstamp_t ts1, int k) {
	tstamp_t ts;
	ts.tv_sec   = ts1.tv_sec  / k;
	ts.tv_usec  = ts1.tv_usec / k;
	ts.tv_usec += (ts.tv_sec % k) * 1000000 / k

	return ts;
}

tstamp_t
tstamp_zero() {
	tstamp_t t = {0, 0};
	return t;
}

tstamp_t
tstamp_get() {
	tstamp_t ts;
	struct timezone tz;
	gettimeofday(&ts, &tz);
	return ts;
}

int
tstamp_cmp(tstamp_t ts1, tstamp_t ts2) {
	if (ts1.tv_sec  < ts2.tv_sec)  return  1;
	if (ts1.tv_sec  > ts2.tv_sec)  return -1;
	if (ts1.tv_usec < ts2.tv_usec) return  1;
	if (ts1.tv_usec > ts2.tv_usec) return -1;
	return 0;
}

char *
tstamp_fmt(char *buff, tstamp_t ts) {
	struct tm *t;
	t = localtime(&ts.tv_sec);
	sprintf(buff, "%02d/%02d/%04d %02d:%02d:%02d.%03d", 
		t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec, ts.tv_usec / 1000);
	return buff;
}

char *
tstamp_duration_fmt(char *buff, tstamp_t ts) {
	struct tm *t;
	t = localtime(&ts.tv_sec);
	
	sprintf(buff, "%d years %d months %d days %02d:%02d:%02d.%03d", 
		t->tm_year - 70, t->tm_mon, t->tm_mday - 1, t->tm_hour - 1, t->tm_min, t->tm_sec, ts.tv_usec / 1000);
	return buff;
}

/* timer part: */
ptmr_t
tmr_new() {
	ptmr_t t;
	if (!(t = (ptmr_t) malloc(sizeof(tmr_t)))) return NULL;

	t->state   = tmrZERO;
	t->start   = tstamp_zero();
	t->elapsed = tstamp_zero();
	
	return t;
}

ptmr_t
tmr_destroy(ptmr_t t) {
	if (t) free(t);
	return NULL;
}

tmr_state_t
tmr_state(ptmr_t t) {
	if (!t) return tmrBADTIMER;
	return t->state;
}

tstamp_t
tmr_elapsed(ptmr_t t) {
	if (!t) return tstamp_zero();
	return t->elapsed;
}

int
tmr_reset(ptmr_t t) {
	if (!t) return tmrBADTIMER;
	t->state = tmrZERO;
	t->start = t->elapsed = tstamp_zero();
	return 0;
}

int
tmr_start(ptmr_t t) {
	if (!t) return tmrBADTIMER;
	if (t->state == tmrPAUSED || t->state == tmrZERO) {
		t->start =  tstamp_get();
		t->state =  tmrRUNNING;
		return 0;
	}
	return tmrERROR;
}
int
tmr_stop(ptmr_t t) {
	if (!t) return tmrBADTIMER;

	if (t->state == tmrRUNNING || t->state == tmrPAUSED) {
		tstamp_t ts;
		ts = tstamp_get();
		t->elapsed = tstamp_add(t->elapsed, tstamp_sub(ts, t->start));
		t->start = tstamp_zero();				
		t->state = tmrSTOPPED;
	}
	return 0;
}

tstamp_t
tmr_pause(ptmr_t t) {
	tstamp_t ts;
	if (!t) return tstamp_zero();

	ts = tstamp_get(); 
	t->elapsed = tstamp_add(t->elapsed, tstamp_sub(ts, t->start));
	t->start = tstamp_zero();
	t->state = tmrPAUSED;
	return t->elapsed;
}

tstamp_t
tmr_laps(ptmr_t t) {
	tstamp_t ts;
	if (!t) return tstamp_zero();

	ts = tstamp_get(); 
	t->elapsed = tstamp_add(t->elapsed, tstamp_sub(ts, t->start));
	t->start   = ts;
	
	return t->elapsed;
}

#ifdef _test_tstmr_
#include <unistd.h>

int main(void) {
	char b[100], b1[100], b2[100];
	ptmr_t t;
	tstamp_t ts, ts1, ts2; 

	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));
	printf("%s\n", tstamp_fmt(b, tstamp_get()));

	ts1 = ts2 = tstamp_zero();
	ts1.tv_sec = 12;
	ts2.tv_sec = 13;
	printf("%s - %s = %s\n", 
		tstamp_duration_fmt(b2, ts2), 
		tstamp_duration_fmt(b1, ts1), 
		tstamp_duration_fmt(b, tstamp_sub(ts2, ts1)));

	printf("%s + %s = %s\n", 
		tstamp_duration_fmt(b2, ts2), 
		tstamp_duration_fmt(b1, ts1), 
		tstamp_duration_fmt(b, tstamp_add(ts2, ts1)));

	ts1 = ts2 = tstamp_zero();
	ts1.tv_sec = 12 * 60;
	ts2.tv_sec = 13 * 60;
	printf("%s - %s = %s\n", 
		tstamp_duration_fmt(b2, ts2), 
		tstamp_duration_fmt(b1, ts1), 
		tstamp_duration_fmt(b, tstamp_sub(ts2, ts1)));

	printf("%s + %s = %s\n", 
		tstamp_duration_fmt(b2, ts2), 
		tstamp_duration_fmt(b1, ts1), 
		tstamp_duration_fmt(b, tstamp_add(ts2, ts1)));


	ts1 = ts2 = tstamp_zero();
	ts1.tv_sec = 12 * 3600;
	ts2.tv_sec = 13 * 3600;
	printf("%s - %s = %s\n", 
		tstamp_duration_fmt(b2, ts2), 
		tstamp_duration_fmt(b1, ts1), 
		tstamp_duration_fmt(b, tstamp_sub(ts2, ts1)));

	printf("%s + %s = %s\n", 
		tstamp_duration_fmt(b2, ts2), 
		tstamp_duration_fmt(b1, ts1), 
		tstamp_duration_fmt(b, tstamp_add(ts2, ts1)));

	ts1 = tstamp_zero();
	ts1.tv_sec = 1;
	printf("%s * %d = %s\n",
		tstamp_duration_fmt(b1, ts1), 2, 
		tstamp_duration_fmt(b, tstamp_mul(ts1, 2)));
		
	printf("%s / %d = %s\n",
		tstamp_duration_fmt(b1, ts1), 4, 
		tstamp_duration_fmt(b, tstamp_div(ts1, 4)));
		


	t = tmr_new();
	printf("start timer\n");
	tmr_start(t);
	printf("sleep(2)\n");
	sleep(2);
	tmr_pause(t);
	printf("elapsed = %s\n", tstamp_duration_fmt(b, tmr_elapsed(t)));
	tmr_reset(t);
	tmr_start(t);
	printf("sleep(10)\n");
	sleep(10);
	tmr_pause(t);
	printf("elapsed = %s\n", tstamp_duration_fmt(b, tmr_elapsed(t)));
	
	tmr_destroy(t);

	printf("elapsed since 1/1/1970 = %s\n", tstamp_duration_fmt(b, tstamp_get()));

	return 0;
}
#endif
