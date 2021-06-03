#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <tbx/err.h>
#include <tbx/fmt.h>
#include <tbx/mem.h>

#include "msg_defs.h"

static char *msg_command_strings[] = {
	"OK",  	 /* msg_OK                   */
	"KO", 	 /* msg_KO                   */
	"BYE",	 /* msg_BYE                  */
	"PUT",   /* msg_PUT                  */
	"GET",   /* msg_GET                  */
	"PIG",   /* msg_PIG                  */
	"POG",   /* msg_POG                  */
	"DUP",   /* msg_DUP                  */
	"SVG",   /* msg_SVG                  */
	"SYT",   /* msg_SYT                  */
	"SYN",   /* msg_SYN                  */
	"RTT",   /* msg_RTT                  */
	"RTN",   /* msg_RTN                  */
	"STT",   /* msg_STT                  */
	"STY",   /* msg_STY                  */
	"TRT",   /* msg_TRT                  */
	"TRN",   /* msg_TRN                  */
	"LCK",   /* msg_LCK                  */
	"ULK",   /* msg_ULK                  */
	"POZ",   /* msg_POZ                  */
	"RUN",   /* msg_RUN					 */
	"BAD"    /* msg_BAD                  */
};

static char *msg_error_strings[] =  {
	"none",					/* merr_NONE              */
	"empty queue",			/* merr_EMPTY_QUEUE       */
	"no memory",            /* merr_NO_MEMORY         */
	"requeue",    			/* merr_REQUEUE           */
	"bad server queue",	    /* merr_BAD_QUEUE_SERVER  */
	"bad client queue",     /* merr_BAD_QUEUE_CLIENT  */
	"bad queue",            /* merr_BAD_QUEUE         */
	"bad queue entry",      /* merr_BAD_QUEUE_ENTY    */
	"queue eror",			/* merr_QUEUE_ERROR       */
	"invalid checksum",	    /* merr_INVALID_CHECKSUM  */
	"invalid protocol",     /* merr_INVALID_PROTOCOL  */
	"null message",         /* merr_NULL_MESSAGE      */
	"incorrect message",    /* merr_INCORRECT_MESSAGE */
	"bad message",          /* merr_BAD_MESSAGE       */
	"partial header",       /* merr_PARTIAL_HEADER    */
	"partial message",      /* merr_PARTIAL_MESSAGE   */
	"max retry reached",    /* merr_MAX_RETRY         */

	"unknown command"       /* merr_UNKNOWN_COMMAND   */
};

static char *msg_action_strings[] = {
	"none",
	"retry",
	"emtpy",
	"bye"
};

static char msg_magic_string[] = "-NQM-MAGIC-HDR->0000";

typedef struct {
	char            magic[16];
	char            pver[4];
	msg_command_t   cmd;
	unsigned int    seq;
	size_t   		size;
} msg_hdr_t, *pmsg_hdr_t;

typedef msg_hdr_t   msg_ok_t;
typedef pmsg_hdr_t  pmsg_ok_t;

typedef struct {
	msg_hdr_t	    hdr;
	msg_ko_action_t action;
	merr_t		    err;
	char            message[];
} msg_ko_t, *pmsg_ko_t;  

typedef struct {
	msg_hdr_t	  hdr;
	unsigned long crc;
	size_t        size;
	char          data[];
} msg_put_t, *pmsg_put_t;

typedef msg_hdr_t   msg_get_t;
typedef pmsg_hdr_t  pmsg_get_t;

typedef union {
	msg_hdr_t hdr;
	msg_ok_t  ok;
	msg_ko_t  ko;
	msg_put_t put;
	msg_get_t get;
} msg_t, *pmsg_t;

#define  _msg_c_
#include "msg.h"
#undef   _msg_c_

char *
msg_act_string(msg_ko_action_t a) {
	return msg_action_strings[a];
}

char *
msg_err_string(merr_t err) {
	return msg_error_strings[err];
}

char *
msg_cmd_string(msg_command_t cmd) {
	return msg_command_strings[cmd];
}

size_t msg_hdr_sizeof() {
	return sizeof(msg_hdr_t);
}

msg_command_t
msg_cmd(pmsg_t msg) {
	if (!msg) return msg_BAD;
	return msg->hdr.cmd;
}

void 
msg_hdr_dump(pmsg_t msg) {
	char m[17], v[5];
	memset(m, 0, 17);
	memset(v, 0, 5);
	strncpy(m, msg->hdr.magic, 16);
	strncpy(v, msg->hdr.pver,   4);
	printf("hdr:\n");
	printf("magic   = %s\n", m);
	printf("version = %s\n", v);
	printf("cmd     = %d (%s)\n", msg_cmd(msg), msg_cmd_string(msg_cmd(msg)));
	printf("size    = %lu\n", msg_load_size(msg)); 
}

void 
msg_ko_dump(pmsg_t msg) {
	pmsg_ko_t ko = (pmsg_ko_t) msg;
	printf("action  = %d (%s)\n", msg_ko_action(msg), msg_act_string(msg_ko_action(msg)));
	printf("err     = %d (%s)\n", msg_ko_err(msg),    msg_err_string(msg_ko_err(msg)));
	printf("message = %s\n", ko->message);
}

void 
msg_put_dump(pmsg_t msg) {
	pmsg_put_t put = (pmsg_put_t) msg;
	printf("crc     = %lu\n", put->crc);
	printf("size    = %lu\n", put->size);
	printf("data    = \n");
	fmt_dump_bin_data(put->data, put->size);
}

void
msg_dump(pmsg_t msg) {
	msg_hdr_dump(msg);
	switch (msg_cmd(msg)) {
	case msg_OK:
	case msg_GET:
		break;
	case msg_KO:
		msg_ko_dump(msg);	
		break;
	case msg_PUT:
		msg_put_dump(msg);	
		break;
	default:
		printf("Unknown message");
	}
	printf("-----\n");
}

size_t
msg_hdr_size(pmsg_t msg) {
	if (!msg) return 0;
	return msg_hdr_sizeof();
}

size_t 
msg_load_size(pmsg_t msg) {
	if (!msg) return 0;
	return msg->hdr.size;
}

size_t
msg_full_size(pmsg_t msg) {
	if (!msg) return 0;
	return msg->hdr.size + msg_hdr_sizeof();

}
unsigned int
msg_seq(pmsg_t msg) {
	if (!msg) return 0;
	return msg->hdr.seq;
}

char *
msg_payload(pmsg_t msg) {
	if (msg && msg->hdr.size) 
		return ((char *) msg) + msg_hdr_sizeof();
	return NULL;
}

static void
_msg_hdr_set(pmsg_hdr_t hdr, msg_command_t cmd, unsigned int seq, size_t size) {
	strncpy(hdr->magic, msg_magic_string, 20);
	hdr->cmd  = cmd;
	hdr->seq  = seq;
	hdr->size = size;
}
int 
msg_check(pmsg_t msg) {
	if (strncmp(msg_magic_string, msg->hdr.magic, strlen(msg_magic_string))) {
		return merr_BAD_MESSAGE;
	}
	return merr_NONE;
}

pmsg_t 
msg_new(pmsg_hdr_t hdr) {
	pmsg_t m;

	if (!hdr) return NULL;
	if (!(m = (pmsg_t) mem_zmalloc(msg_hdr_sizeof() + hdr->size))) return NULL;
	memcpy((char *) m, (char *) hdr, msg_hdr_sizeof());

	err_debug("<<<<");
	msg_hdr_dump(m);
	err_debug(">>>>");

	return m;
}

pmsg_t 
msg_hdr_new(msg_command_t cmd, unsigned int seq, unsigned int size) {
	pmsg_hdr_t hdr;
	if (!(hdr = malloc(sizeof(msg_hdr_t)))) return NULL;
	
	_msg_hdr_set(hdr, cmd, seq, size);	
	return (pmsg_t) hdr;
}

pmsg_t
msg_ok_new(unsigned int seq) {
	return (pmsg_t) msg_hdr_new(msg_OK, seq, 0);
}

pmsg_t
msg_ko_new(unsigned int seq, merr_t err, int action, char *message) {
	pmsg_ko_t  ko;

	size_t sz = sizeof(msg_ko_t) + strlen(msg_error_strings[err]) + 1;

	if (!(ko = (pmsg_ko_t) malloc(sz))) return NULL;

	_msg_hdr_set((pmsg_hdr_t) ko, msg_KO, seq, sz - sizeof(msg_hdr_t));	
	ko->action   = action;
	ko->err      = err;
	strcpy(ko->message, message);

	return (pmsg_t) ko; 
}

pmsg_t
msg_put_new(unsigned int seq, char *data, size_t size, unsigned long crc) {
	pmsg_put_t  put;

	size_t sz = sizeof(msg_put_t) + size;

	err_debug("put msg size hdr %lu + load %lu + data %lu = %lu (%lu)", 
		sizeof(msg_hdr_t),
		sizeof(msg_put_t) - sizeof(msg_hdr_t),
		size, 
		sizeof(msg_put_t) + size,
		sz);
		
	if (!(put = (pmsg_put_t) mem_zmalloc(sz))) return NULL;

	_msg_hdr_set((pmsg_hdr_t) put, msg_PUT, seq, sz - sizeof(msg_hdr_t));	
	put->crc  = crc;
	put->size = size;

	memcpy(put->data, data, size);

	msg_dump((pmsg_t) put);
	return (pmsg_t) put; 
} 
pmsg_t
msg_get_new(unsigned int seq) {
	return (pmsg_t) msg_hdr_new(msg_GET, seq, 0);
}

pmsg_t
msg_destroy(pmsg_t msg) {
	if (msg) free(msg);
	return NULL;
}

int
msg_seq_incr(pmsg_t msg) {
	if (!msg) return 0;
	return msg->hdr.seq++;
}

char *
msg_put_data(pmsg_t msg) {
	if (!msg || msg->hdr.cmd != msg_PUT) return NULL;	
	return msg->put.data;
}

size_t
msg_put_size(pmsg_t msg) {
	if (!msg || msg->hdr.cmd != msg_PUT) return 0;	
	return msg->put.size;
}

unsigned long
msg_put_crc(pmsg_t msg) {
	if (!msg || msg->hdr.cmd != msg_PUT) return 0;	
	return msg->put.crc;
}

merr_t
msg_ko_err(pmsg_t msg) {
	if (!msg || msg->hdr.cmd != msg_KO) return merr_NONE;	
	return msg->ko.err;
}
char *
msg_ko_err_string(pmsg_t msg) {
	if (!msg || msg->hdr.cmd != msg_KO) return NULL;	
	return msg_error_strings[msg->ko.err];
}

msg_ko_action_t
msg_ko_action(pmsg_t msg) {
	if (!msg || msg->hdr.cmd != msg_KO) return act_NONE;	
	return msg->ko.err;
}

char *
msg_ko_message(pmsg_t msg) {
	if (!msg || msg->hdr.cmd != msg_KO) return NULL;	
	return msg->ko.message;
}

