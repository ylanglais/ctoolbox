#include <stdlib.h>
#include <string.h>

#include <tbx/rexp.h>

/** SMS part: **/

typedef struct {
	char src[16];
	char dst[16];
	char msg[161];
	char id[66];
	int status;
} sms_t, *psms_t;

psms_t 
sms_destroy(psms_t sms) {
	if (sms) {
		memset((char *) sms, 0, sizeof(sms_t));
		free(sms);
	}
	return NULL;
}

psms_t
sms_new(char *src, char *dst, char *msg) {
	psms_t sms = NULL;
	if (!(sms = (psms_t) malloc(sizeof(sms_t)))) return NULL;	
	memset((char *) sms, 0, sizeof(sms_t)),
	strncpy(sms->src, src, 15);
	strncpy(sms->dst, dst, 15);
	strncpy(sms->msg, msg, 160);
	return sms;
}

int
sms_status_set(psms_t sms, int status) {
	if (!sms) return 1;
	sms->status = 0;
	return 0;
}

int 
sms_id_set(psms_t sms, char *id) {
	if (!sms) return 1;
	strncpy(sms->id, id, 160);
	return 0;
}

int
sms_is_rpt(psms_t s) {
	if (!s) return 0;
	if (strstr(s->msg, "id:") == s->msg) return 0;
	return 1;
}

/*
prpt_t
sms_rpt_get(psms_t sms, prpt_t rpt) {
	char *findstr = "^id:([0-9]{10})[ ]* sub:([0-9]{3})[ ]* dlvrd:([0-9]{3})[ ]* submit date:([0-9]{10})[ ]* done date:([0-9]{10})[ ]* stat:([A-Z]{0,7})[ ]* err:([0-9]{3})[ ]* text:(.{0,20})[ ]*$";

	prexp_t r;

	if (!sms || !rpt) return NULL;
	if (!(r = rexp_new(sms->msg, findstr, rexp_EXTENDED | rexp_NL_DO_NOT_MATCH))) return NULL;
	if (rexp_find(r)) {
		rpt->id     = rexp_sub_get(r, 1);
		rpt->sub    = rexp_sub_get(r, 2);
		rpt->dlvrd  = rexp_sub_get(r, 3);
		rpt->submit = rexp_sub_get(r, 4);
		rpt->done   = rexp_sub_get(r, 5);
		rpt->status = rexp_sub_get(r, 6);
		rpt->err    = rexp_sub_get(r, 7);
		rpt->msg    = rexp_sub_get(r, 8);
		rexp_destroy(r);
		return rpt;
	}
	rexp_destroy(r);
	return NULL;
}
*/
