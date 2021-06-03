

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifndef __REENTRANT__
#define __REENTRANT__
#endif 

#ifdef _threaded_campain_
#include <pthread.h>
#endif

#include <tbx/err.h>
#include <tbx/storage.h>
#include <tbx/hash.h>
#include <tbx/tstmr.h>
#include <tbx/rexp.h>

#include "smpp.h"
#include "ubterm.h"

#define smsc_host "SMPP6.mblox.com"
#define smsc_port 3217
#define smsc_login "Prologue"
#define smsc_pass "bC2oUzS5"

#define dbserver "localhost"
#define dbport	0
#define dbuser	"smsc"
#define dbpass	"smsc_admin"
#define dbname	"SMSc"

/*
#include <db.h>
*/

enum {
	per_unknown,
	per_intermediate,
	per_temporary,
	per_permanent
};

char *mblox_perenity_names[] = {
	"unknown",
	"intermediate",
	"temporary",
	"permanent"
};

enum {
	stat_unknown,
	stat_new,
	stat_sent,
	stat_failed,
	stat_buffered,
	stat_acked,
	stat_delivered
};

char *mblox_status_names[] = {
	"unknown",
	"new",
	"sent",
	"failed",
	"buffered",
	"acked",
	"delivered"
};


typedef struct {
	char id[20];
	char to[20];
	int  perenity;
	int  status;
	tstamp_t sent;
	tstamp_t acked;
	tstamp_t final;
} sms_t, *psms_t;

void
sms_dump(psms_t sms) {
	char b1[50], b2[50], b3[50];
	err_debug(
		"%s;%s;%s;%s;%s;%s;%s", 
		sms->id,
		sms->to,
		mblox_perenity_names[sms->perenity],
		mblox_status_names[sms->status],
		tstamp_fmt(b1, sms->sent),
		tstamp_fmt(b2, sms->acked),
		tstamp_fmt(b3, sms->final)
	);
}

typedef struct {
	tstamp_t 	start;
	tstamp_t 	stop;
	pstorage_t 	store;
	phash_t 	htel;
	phash_t 	hmid;
	char 		from[21];
	char 		text[161];
 	int 		n;
	int 		nsent;
	int 		nfinal;
	int 		nfailed;
	int 		nacked;
	int 		ndelivered;
#ifdef _threaded_campain_
	pthread_mutex_t  *mp;
#endif
} campain_t, *pcampain_t;


#ifdef _threaded_campain_
#define campain_lock(c)   pthread_mutex_lock((c)->mp)  
#define campain_unlock(c) pthread_mutex_unlock((c)->mp) 
#endif

enum {
	mblox001,
	mblox002,
	mblox003,
	mblox004,
	mblox005,
	mblox006,
	mblox008,
	mblox020,
	mblox021,
	mblox023,
	mblox024,
	mblox025,
	mblox026,
	mblox027,
	mblox028,
	mblox029,
	mblox033,
	mblox073,
	mblox074,
	mblox076,
	mblox202
};

int mblox_codes[] = { 1, 2, 3, 4, 5, 6, 8, 20, 21, 23, 24, 25, 26, 27, 28, 29, 33, 73, 74, 76, 202 };

int mblox_perenity[] = {
	per_intermediate,
	per_intermediate,
	per_intermediate,
	per_permanent,
	per_unknown,
	per_unknown,
	per_temporary,
	per_permanent,
	per_temporary,
	per_permanent,
	per_temporary,
	per_temporary,
	per_temporary,
	per_permanent,
	per_permanent,
	per_permanent,
	per_permanent,
	per_permanent,
	per_temporary,
	per_permanent,
	per_permanent
};

int mblox_status[] = {
	stat_buffered,
	stat_buffered,
	stat_acked,
	stat_delivered,
	stat_failed,
	stat_unknown,
	stat_failed,
	stat_failed,
	stat_failed,
	stat_failed,
	stat_failed,
	stat_failed,
	stat_failed,
	stat_failed,
	stat_failed,
	stat_failed,
	stat_failed,
	stat_failed,
	stat_failed,
	stat_failed,
	stat_failed
};

char *mblox_txts[] = {
	"Phone related",
	"Deliverer related: message within operator, retrying",
	"Accepted by the Operator",
	"Message delivered to mobile device",
	"Message delivery failed confirmed by operator without reason",
	"Status of message is unknown (no operator reason)",
	"Message expired prior delivery",
	"Permanent operator error",
	"Credit related: Message has been accepted by operator prior credit check",
	"Absent subscriber permanent",
	"Absent subscriber temporary",
	"Operator network failure",
	"Phone related error",
	"Phone related error",
	"Anti-spam (stop SMS)",
	"Content related error (content not permitted on network/shortcode)",
	"Age verification failure/Parental lock for adult content",
	"Mobile number portability error",
	"MSISDN Roaming error",
	"Mobile number portability and blocking error",
	"Mobile number portability error"
};

void
mblox_test() {
	int i;
	for (i = 0; i <= mblox202; i++) {
		printf("%d;%03d;%s;%s;%s\n", i, mblox_codes[i], 
			mblox_status_names[mblox_status[i]],
			mblox_perenity_names[mblox_perenity[i]],
			mblox_txts[i]);
	}
}

int
mblox_code_explode(int code, int *perenity, int *status, char **txt) {
	int i;
	for (i = 0; code != mblox_codes[i] && i <= mblox202; i++); 
	if (code != mblox_codes[i]) return 1; 
	*perenity = mblox_perenity[i]; 
	*status   = mblox_status[i]; 
	if (txt) 
		*txt  = mblox_txts[i];
	return 0;
}


pcampain_t 
campain_destroy(pcampain_t c) {
	if (c) {
		if (c->hmid)  c->hmid  = hash_destroy(c->hmid);
		if (c->htel)  c->htel  = hash_destroy(c->htel);
		if (c->store) c->store = storage_destroy(c->store);
		free(c);
	}
	return NULL;
}

pcampain_t
campain_new(char *from, char *text) {
	pcampain_t c;
	if (!(c = (pcampain_t) malloc(sizeof(campain_t)))) return NULL;

	memset((char *) c, 0, sizeof(campain_t));

	if (!(c->store = storage_new(sizeof(sms_t), 100))) {
		err_error("cannot create storage");
		return campain_destroy(c);
	}
	if (!(c->hmid = hash_new())) {
		err_error("cannot create msgid hash");
		return campain_destroy(c);
	}
	if (!(c->htel = hash_new())) {
		err_error("cannot create tel hash");
		return campain_destroy(c);
	}
	strncpy(c->from, from, 20);
	strncpy(c->text, text, 160);
	c->from[20]  = 0;
	c->text[160] = 0;
	return c;
}

int
campain_num_add(pcampain_t c, char *to) {
	sms_t sms;
	char *p;
	if (!c) {
		err_error("no campain pointer");
		return 1;
	}
	if (!c->store) {
		err_error("no campain store");
		return 2;
	} 
	if (!c->htel) {
		err_error("no campain has for telephone numbers");
		return 3;
	}
	memset((char *) &sms, 0, sizeof(sms_t));
	strcpy(sms.to, to);
	sms.status = stat_new;
	if (!(p = storage_add(c->store, (char *) &sms))) {
		err_error("cannot insert sms in campain store");
		return 4;
	}
	if (hash_insert(c->htel, to, p)) {
		err_error("cannot index sms in campain tel index");
		return 5;
	}
	c->n++;
	return 0;
}

int
campain_receipt_parse(char *msg, char *id, int *status, int *perenity) {
	static char findstr[] = "^id:([0-9]{10})[ ]* sub:([0-9]{3})[ ]* dlvrd:([0-9]{3})[ ]* submit date:([0-9]{10})[ ]* done date:([0-9]{10})[ ]* stat:([A-Z]{0,7})[ ]* err:([0-9]{3})[ ]* text:(.{0,20})[ ]*$";
	prexp_t r;

	if (!(r = rexp_new(msg, findstr, rexp_EXTENDED | rexp_NL_DO_NOT_MATCH)) || !rexp_find(r)) {
		err_error("regular expression error / receipt fields not found");
		return 1;
	} else { 
		char *ident, *state, *error;
		int errcode;

		ident = rexp_sub_get(r, 1);
		state = rexp_sub_get(r, 6);
		error = rexp_sub_get(r, 7);

		strcpy(id, ident);
		errcode = atoi(error);

		if (!strcmp("DELIVRD", state)) {
			*perenity = per_permanent;
			*status   = stat_delivered;
		} else {
			mblox_code_explode(errcode, perenity,           status, (char **) NULL);
			//mblox_code_explode(errcode, int *perenity, int *status, char **txt) {
			
		}
		free(ident); free(error); free(state);
	}
	return 0;
}

void
campain_rt_status(pcampain_t c) {
	char tsbuf[20];
	int pcent, i;
	tstamp_t now, start;
	now = tstamp_get();
	start = now;
	printf("Campain statistics at %s\n---------------------------------------------\n", tstamp_fmt(tsbuf, now)); 
	printf("Started         :  %s\n", tstamp_fmt(tsbuf, c->start));
	if (tstamp_cmp(c->stop, tstamp_zero())) 
	printf("Ended           :  %s\n", tstamp_fmt(tsbuf, c->stop));
	else
	printf("Ended           :      n/a\n");
	printf("Elapsed         :  %s                          \n", tstamp_duration_fmt(tsbuf, tstamp_sub(now, c->start)));
	printf("#sms            :  %7d\n", c->n);
	printf("#sms sent       :  %7d\n", c->nsent);
	printf("#sms acked      :  %7d\n", c->nacked);
	printf("#sms final      :  %7d\n", c->nfinal);
	printf("#sms failed     :  %7d\n", c->nfailed);
	printf("#sms delivered  :  %7d\n", c->ndelivered);
	pcent = 100 * c->nsent / c->n;
	for (i = 0; i < 20 && i < pcent / 5; i++) tsbuf[i] = '=';
	tsbuf[i] = 0;
	printf("percentage sent :  %7d |%-20s|\n", pcent, tsbuf);
	pcent = 100 * c->nacked / c->n;
	for (i = 0; i < 20 && i < pcent / 5; i++) tsbuf[i] = '=';
	tsbuf[i] = 0;
	printf("percentage ackd :  %7d |%-20s|\n", pcent, tsbuf);
	pcent = 100 * c->nfinal / c->n;
	for (i = 0; i < 20 && i < pcent / 5; i++) tsbuf[i] = '=';
	tsbuf[i] = 0;
	printf("percentage done :  %7d |%-20s|\n", pcent, tsbuf);
}

void *
campain_stats(void *cc) {
	pcampain_t c;
	c = (pcampain_t) cc;

	while (c->nfinal < c->n || c->nacked < c->n) {
		usleep(10000);
		campain_rt_status(c);
		printf("\033[14A");
	}
	printf("\033[14B");
	return NULL;
}

void
campain_final_status(pcampain_t c) {
	//printf("\33[14B");
	printf("\n*** Campain sent ***\n");
	campain_rt_status(c);
	printf("\33[14B");
}

void 
campain_sms_status(pcampain_t c) {
	/* final stats for sms (ack time, final time, ...) */
	int i;
	tstamp_t acked, final, final_ok, final_ko;

	int nfinal, nacked, nfinal_ok, nfinal_ko;
	nfinal = nacked = nfinal_ok = nfinal_ko = 0;
	acked = final = final_ok = final_ko = tstamp_zero();

	for (i = 0; i < storage_used(c->store); i++) {
		psms_t sms;
		sms = (psms_t) storage_get(c->store, i);

		sms_dump(sms);

		if (tstamp_cmp(sms->acked, sms->sent) > 0) {
			acked = tstamp_add(acked, tstamp_sub(sms->acked, sms->sent));
			nacked++;
		}
		final = tstamp_add(final, tstamp_sub(sms->final, sms->sent));
		nfinal++;
		if (sms->status == stat_failed) {
			final_ko = tstamp_add(final, tstamp_sub(sms->final, sms->sent));
			nfinal_ko++;
		} else if (sms->status == stat_delivered) {
			final_ok = tstamp_add(final, tstamp_sub(sms->final, sms->sent));
			nfinal_ok++;
		}
	}
	char tsbuf[500];
	printf("\nSMS statistics:\n---------------\n");
	printf("percentage ok             : %7d\n", nfinal_ok * 100 / c->n);
	printf("percentage ko             : %7d\n", nfinal_ko * 100 / c->n);
	printf("percentage acked          : %7d\n", nacked    * 100 / c->n);
	printf("#sms acked                : %7d\n", nacked);
	printf("#sms nfinal_ok            : %7d\n", nfinal_ok);
	printf("#sms nfinal_ko            : %7d\n", nfinal_ko);
	if (nacked) 
	printf("time before acked         : %s\n",  tstamp_duration_fmt(tsbuf, tstamp_div(acked, nacked)));
	if (nfinal) 
	printf("time before final         : %s\n",  tstamp_duration_fmt(tsbuf, tstamp_div(final, nfinal)));
	if (nfinal_ok) 
	printf("time before final when OK : %s\n",  tstamp_duration_fmt(tsbuf, tstamp_div(final_ok, nfinal_ok)));
	if (nfinal_ko) 
	printf("time before final when KO : %s\n",  tstamp_duration_fmt(tsbuf, tstamp_div(final_ko, nfinal_ko)));
}

#ifdef _threaded_campain_
void *
#else 
int
#endif
campain_update(void *cc) {
	psmpp_t in;
	pcampain_t c;
		
	if (!cc) {
		err_error("no campain pointer :^(");
#ifdef _threaded_campain_
		pthread_exit(NULL);
#else
		return 1;
#endif
	}
	c = (pcampain_t) cc;
	
	if (!(in = smpp_new(smsc_host, smsc_port))) {
		err_error("cannot create smpp receiver"); 
#ifdef _threaded_campain_
		pthread_exit(NULL);
#else 
		return 2;
#endif
	}
	smpp_bind(in, smppRECEIVER, smsc_login, smsc_pass);
	
	err_info("Campain contains %d sms (%d processed yet)", c->n, c->nfinal);

#ifdef _threaded_campain_
	/* give sender a bit of advance... */
	usleep(500);
#endif

	while (c->nfinal < c->n || c->nacked < c->n) {
		ppdu_t p;
		char pdu[PDUSIZE+1];
		smpp_read_pdu(in, pdu);
		p = (ppdu_t) pdu;
		if (ntohl(p->command_id) != deliver_sm) {
			err_info("Recieved command %08x", ntohl(p->command_id));
		} else {
			char id[30];
			char from[21] , to[21], message[200];
			int status, perenity;
			psms_t sms;

		//	err_info("Recieved command %08x", ntohl(p->command_id));

			smpp_sms_parse(in, pdu, from, to, message);
			campain_receipt_parse(message, id, &status, &perenity);
			err_info("text = \"%s\"", message);
			err_info("sms %s to %s is %s perenity: %s", 
				id, to, mblox_status_names[status], mblox_perenity_names[perenity]); 

			if (!(sms = hash_retrieve(c->hmid, id))) {
				err_error("cannot find message %s in current campain", id);
			} else {
				if  (status == stat_acked) {
					sms->acked = tstamp_get();
					c->nacked++;
					if (sms->perenity != per_permanent && (
						sms->status != stat_delivered || 
						sms->status != stat_failed)) {
						sms->perenity = perenity;
						sms->status   = status;		
					}
				} else if (status == stat_failed || status == stat_delivered) {
					sms->status   = status;
					sms->perenity = perenity;
					sms->final    = tstamp_get();
					c->nfinal++;
					if   (status == stat_failed)    c->nfailed++;
					else c->ndelivered++;
				} else if (sms->perenity < perenity) {
					sms->status   = status;
					sms->perenity = perenity;
				}	
#ifdef _threaded_campain_
#else
				campain_rt_status(c);
				printf("\033[14A");
#endif
			}
		}
	}	
	c->stop = tstamp_get();

	smpp_destroy(in);	
#ifdef _threaded_campain_
	pthread_exit(NULL);
	return NULL;
#else
	return 0;
#endif
}

int
campain_send(pcampain_t c) {
	int i;
	psms_t sms;
	psmpp_t out;
	
	if (!(out = smpp_new(smsc_host, smsc_port))) return 1;
	smpp_bind(out, smppTRANSMITTER, smsc_login, smsc_pass);
	
	c->start = tstamp_get();
	for (i = 0; i < storage_used(c->store); i++) {
		sms = (psms_t) storage_get(c->store, i);
		smpp_submit_sm(out, c->from, sms->to, c->text, sms->id);
		sms->sent = tstamp_get();
		sms_dump(sms);
		if (hash_insert(c->hmid, sms->id, (char *) sms)) {
			err_error("cannot index sms msg id %s campain", sms->id);
			continue;
		}
		c->nsent++;
		err_info("Sent sms %s to %s", sms->id, sms->to);
	}
	smpp_destroy(out);	
	return 0;
}

int
main(int n, char *a[]) {
#ifdef _test_mblox_ref_
	mblox_test();
	return 0;
#else	
	pcampain_t c;
	int i, count; 
#ifdef _threaded_campain_
	pthread_t updater, stats;
#endif

/****
{
	char id[50];
	int perenity, status;

	campain_receipt_parse("id:1600077961 sub:001 dlvrd:001 submit date:1303151251 done date:1303151251 stat:ACKED   err:003 text: ", id, &status, &perenity);
	exit(0);
}
*****/

	err_init("./log", err_DEBUG);
	ubterm_ini();
	
	if (!(c = campain_new("01010", "Perf testing"))) {
		err_error("no campain created, aborting");
		return 1;
	}

	if (n == 1) count = 1;
	else count = atoi(a[1]);
	if (count < 1) count = 1;
	else if (count > 100000) count = 100000;

	err_info("count is %d", count);

	for (i = 0; i < count; i++) {
		char b[15];
		sprintf(b, "336666%06d", i);
		err_info("%d adds %s", i, b);
		if (campain_num_add(c, b));
	}
	
	err_info("Campain has %d sms to send", c->n, c->nfinal);

#ifdef _threaded_campain_
	if ((pthread_create(&updater, NULL, campain_update, (void *) c))) {
		err_error("cannot create updater thread : %s", strerror(errno));
	}
	if ((pthread_create(&stats, NULL, campain_stats, (void *) c))) {
		err_error("cannot create stats thread : %s", strerror(errno));
	}
#endif
	campain_send(c);
#ifdef _threaded_campain_
	pthread_join(updater, NULL);
	pthread_join(stats,   NULL);
	pthread_exit(NULL);
#else
	campain_update(c);
#endif

	campain_final_status(c);
	campain_sms_status(c);
	campain_destroy(c);

	ubterm_fini();
	return 0;
#endif
}
