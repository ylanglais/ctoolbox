/*
	This code is released under ...  terms. 
	Please read license terms and conditions at ... 

	Yann LANGLAIS

	Changelog:
	13/03/2013 0.9.1 initial release / partial smpp 3.4 support (read/write sms)
*/

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>

#ifdef __REENTRANT__
#include <pthread.h> 
#define _smpp_lock(x)   pthread_mutex_lock((x)->mp)
#define _smpp_unlock(x) pthread_mutex_unlock((x)->mp)
#else 
#define _smpp_lock(x) 
#define _smpp_unlock(x) 
#endif

#include <tbx/rexp.h>
#include <tbx/cli.h>
#include <tbx/err.h>
#include <tbx/tstmr.h>

#include "smpp_def.h"
#include "sms.h"

char smpp_NAME[]    = "SMPP management";
char smpp_VERSION[] = "1.0.0";
char smpp_DATEVER[] = "08/04/2013";

typedef int (*f_check_t)(void *, char *);
typedef int (*f_update_t)(void *, char *, int);

typedef struct {
	pcli_t		cli;
	esme_type_e	type;	
	int         status;	
	int         seq;
	f_check_t   f_check;	
	f_update_t  f_update; ;	
	char *      valid_rexp;
	#ifdef __REENTRANT__
	pthread_mutex_t  *mp;
	#endif
} smpp_t, *psmpp_t;

#define  _smpp_c_
#include <smpp.h>
#undef   _smpp_c_

char *smppCOMMANDS[] = {
	"nac: no command",
	"bind_receiver",
	"bind_transmitter",
	"query_sm",
	"submit_sm",
	"deliver_sm",
	"unbind",
	"replace_sm",
	"cancel_sm",
	"bind_transceiver",
	"outbind",
	"enquire_link",
	"submit_multi",
	"alert_notification",
	"data_sm"
};

char *smppCOMMANDS_RESP[] = {
	"generic_nack",
	"bind_receiver_resp",
	"bind_transmitter_resp",
	"query_sm_resp",
	"submit_sm_resp",
	"deliver_sm_resp",
	"unbind_resp",
	"replace_sm_resp",
	"cancel_sm_resp",
	"bind_transceiver_resp",
	"nac: outbind nack",
	"enquire_link_resp",
	"submit_multi_resp",
	"nac: alert_notification nack",
	"data_sm_resp"
};

static char smppINVALID_CMD[] = "invalid command";


char *
smpp_cmd_name(int id) {
	char **a;

	if (id & (generic_nack)) {
		id ^= (generic_nack);
		a = smppCOMMANDS_RESP;
	} else {
		a = smppCOMMANDS;
	}

	if (id > 10) switch (id) {
	case outbind:
		id = 10;
		break;
	case enquire_link:
		id = 11;
		break;
	case submit_multi:
		id = 12;
		break;
	case alert_notification:
		id = 13;
		break;
	case data_sm:
		id = 14;
		break; 
	default:
		return smppINVALID_CMD;
	}		
	return a[id];	
}

psmpp_t
smpp_destroy(psmpp_t s) {
	if (s) {
		if (s->cli) {
			/* need to unbind ! */
			//smpp_unbind(s);
			s->cli = cli_destroy(s->cli);
		}
		free(s);	
	}
	return NULL;
}

psmpp_t 
smpp_new(char *host, int port) {
	psmpp_t s;

	if (!(s = (psmpp_t) malloc(sizeof(smpp_t)))) {
		err_error("no memory for smpp structure");
		return NULL;
	}
	
	if (!(s->cli = cli_new(host, port))) {
		err_error("cannot create socket for host %s:%d", host, port);
		return smpp_destroy(s);	
	}

	// cli_recv_opts_set(s->cli, MSG_WAITALL | MSG_TRUNC);
	
	s->seq = 1; //getpid() % 255;

	//smpp_bind(s, login, passwd);

	return s;	
}

int
smpp_bind(psmpp_t smpp, esme_type_e type, char *login, char *passwd) {
	char pdu[PDUSIZE + 1];
	char res[PDUSIZE + 1];
	char *p, *q;
	ppduhdr_t hdr;
	memset(pdu, 0, PDUSIZE);
	memset(res, 0, PDUSIZE);

	hdr = (ppduhdr_t) pdu;

	hdr->command_id = htonl(type);
	hdr->seq_num    = htonl(smpp->seq++);

	p = pdu + sizeof(pduhdr_t);
	q = login;  while ((*p++ = *q++)); // login
	q = passwd; while ((*p++ = *q++)); // passwd
	q = "WWW";  while ((*p++ = *q++)); // ESME TYPE = WWW 
	*p++ = 0x34; 		               // smpp version (char)
	*p++ = 0;                          // addr_ton     (char)
	*p++ = 0;                          // addr_npi     (char)
	*p++ = 0;	                       // address_range (null string)
	hdr->command_len = htonl(p - pdu);
	
	_smpp_lock(smpp);
	smpp_sen(smpp, pdu);
	smpp_rec(smpp, res);
	_smpp_unlock(smpp);
	
	if (pdu_cmd((ppdu_t) res) != (pdu_cmd((ppdu_t) pdu) | generic_nack)) {
		err_error("waited for %08x and recieved %08x", (pdu_cmd((ppdu_t) pdu) | generic_nack), pdu_cmd((ppdu_t) res));
		return -3;
	}	
	if (pdu_seq((ppdu_t) res) != (pdu_seq((ppdu_t) pdu))) {
		err_error("waited for seq %d and recieved %d", pdu_seq((ppdu_t) pdu), pdu_seq((ppdu_t) res));
		return -4;
	}

	/* check return status */
	return pdu_status((ppdu_t) res);
}

int
smpp_unbind(psmpp_t smpp) {
	char pdu[PDUSIZE + 1];
	char res[PDUSIZE + 1];
	ppduhdr_t hdr;
	memset(pdu, 0, PDUSIZE);
	memset(res, 0, PDUSIZE);
	
	hdr = (ppduhdr_t) pdu;

	hdr->command_id  = htonl(unbind);
	hdr->seq_num     = htonl(smpp->seq++);
	hdr->command_len = htonl(sizeof(pduhdr_t));
		
	_smpp_lock(smpp);
	smpp_sen(smpp, pdu);
	smpp_rec(smpp, res);
	_smpp_unlock(smpp);
	
	if (pdu_cmd((ppdu_t) res) != (pdu_cmd((ppdu_t) pdu) | generic_nack)) {
		err_error("waited for %08x and recieved %08x", (pdu_cmd((ppdu_t) pdu) | generic_nack), pdu_cmd((ppdu_t) res));
		return -3;
	}	
	if (pdu_seq((ppdu_t) res) != (pdu_seq((ppdu_t) pdu))) {
		err_error("waited for seq %d and recieved %d", pdu_seq((ppdu_t) pdu), pdu_seq((ppdu_t) res));
		return -4;
	}


	/* check return status */
	return pdu_status((ppdu_t) res);
}

int
smpp_outbind(psmpp_t s) {
	err_error("function not implemented");
	return 0;
}

int
smpp_query_sm(psmpp_t s) {
	err_error("function not implemented");
	return 0;
}

int
smpp_submit_sm(psmpp_t s, char *from, char *to, char *message, char *id) {
	char pdu[PDUSIZE + 1];
	char res[PDUSIZE + 1];
	char *p, *q;
	ppduhdr_t hdr;
	memset(pdu, 0, PDUSIZE);
	memset(res, 0, PDUSIZE);
	
	hdr = (ppduhdr_t) pdu;

	hdr->command_id  = htonl(submit_sm);
	hdr->seq_num     = htonl(s->seq++);

	p = pdu + sizeof(pduhdr_t);
	/* service type (0 to 6 chars + null char) => here = "" */
	p++;
	/* source_addr_ton = 0*/
	p++;
	/* source_addr_npn = 0*/
	p++;
	/* source */
	q = from; while ((*p++ = *q++));
	/* dest_addr_ton = 0*/
	p++;
	/* dest_addr_npn = 0*/
	p++;
	/* dest */
	q = to;   while ((*p++ = *q++));	
	/* sms_esm_class  = 0 if single = 0x40 if multiple */
	p++;
	/* sms_protocol_id = 0*/
	p++;
	/* sms_priority_flag = 0*/
	p++;
	/* sms_schedule_delivery_time date/time of delivery schedule (NULL = immadiate)*/
	p++;
	/* sms_validity_period validity duration (NULL = SMSC default) */
	p++;
	/* sms_registered_delivery_flag aka need receipt flag : 0 -> no, 1 -> yes */
	*p++ = 1;
	/* sms_replace_if_present_flag : 0 -> no, 1 -> yes */
	p++;
	/* sms_data_coding encoding scheme (?) => 0 */
	p++;
	/* sms_sm_default_msg_id if smc defines "canned" message => 0 for specific message */
	p++;	
	/* message length */
	*p++ = (char) strlen(message), 
	/* Message */
	q = message; while ((*p++ = *q++));
 
	hdr->command_len = htonl(p - pdu);
		
	_smpp_lock(s);
	smpp_sen(s, pdu);
	smpp_rec(s, res);
	_smpp_unlock(s);

	if (pdu_cmd((ppdu_t) res) != (pdu_cmd((ppdu_t) pdu) | generic_nack)) {
		err_error("waited for %08x and recieved %08x", (pdu_cmd((ppdu_t) pdu) | generic_nack), pdu_cmd((ppdu_t) res));
		return -3;
	}	
	if (pdu_seq((ppdu_t) res) != (pdu_seq((ppdu_t) pdu))) {
		err_error("waited for seq %d and recieved %d", pdu_seq((ppdu_t) pdu), pdu_seq((ppdu_t) res));
		return -4;
	}

	p = res + sizeof(pdu_t);

	long l;
	sscanf(p,   "%x", &l);
	sprintf(id, "%d",  l);	
	if (getenv("SMPPDBG")) err_debug("message_id = Ox%s, %u", p, l);

	/* check return status */
	return pdu_status((ppdu_t) res);
}

int
smpp_deliver_sm(psmpp_t s) {
	err_error("function not implemented");
	return 0;
}

int
smpp_replace_sm(psmpp_t s) {
	err_error("function not implemented");
	return 0;
}

int
smpp_cancel_sm(psmpp_t s) {
	err_error("function not implemented");
	return 0;
}

int
smpp_enquire_link(psmpp_t s) {
	err_error("function not implemented");
	return 0;
}

int
smpp_submit_multi(psmpp_t s) {
	err_error("function not implemented");
	return 0;
}

int
smpp_alert_notification(psmpp_t s) {
	err_error("function not implemented");
	return 0;
}

int
smpp_data_sm(psmpp_t s) {
	err_error("function not implemented");
	return 0;
}


int
smpp_sen(psmpp_t s, char *pdu) {
	if (!s || !s->cli || !pdu) return -1;

	if (getenv("SMPPDBG")) {
		if (getenv("SMPPDBG")) {
			err_info("Sending    command %08x: %s", pdu_cmd((ppdu_t) pdu), smpp_cmd_name(pdu_cmd((ppdu_t) pdu)));
			pdu_hexdump((ppdu_t) pdu);
		}
	}

	cli_send(s->cli, (char *) pdu, pdu_len((ppdu_t) pdu));
	return 0;
}

int 
smpp_rec(psmpp_t s, char *pdu) {
	if (!s) {
		err_error("invalid smpp pointer");
		return -1;
	}
	if (!s->cli) {
		err_error("invalid smpp cli pointer");
		return -2;
	}
	if (!pdu) {
		err_error("invalid pdu pointer");
		return -3;
	}
	if (!(cli_recv(s->cli, pdu, PDUSIZE))) {
		err_error("bad pdu response");
		return -4;
	}
	if (getenv("SMPPDBG")) err_info("Receiveing command %08x: %s", pdu_cmd((ppdu_t) pdu), smpp_cmd_name(pdu_cmd((ppdu_t) pdu)));
	pdu_hexdump((ppdu_t) pdu);
	return 0;
}

int
smpp_sms_parse(psmpp_t s, char *pdu, char *from, char *to, char *message) {
	char *p;
	//char  sms_registered_delivery_flag;
	size_t len;
	int i;
	ppdu_t h;

	if (!pdu) return 1;
	if (!from || !to || !message) return 2;

	h = (pdu_t *) pdu;

	p = pdu + sizeof(pduhdr_t);
	/* skip service type (0 to 6 chars + null char) */
	while (*p) p++;
	/* skip source_addr_ton = 0*/
	p++;
	/* skip source_addr_npn = 0*/
	p++;
	/* source */
	strncpy(from, p, 19); 
	from[20] = 0;
	while (*p++);
	/* dest_addr_ton = 0*/
	p++;
	/* dest_addr_npn = 0*/
	p++;
	/* dest */
	strncpy(to, p, 19);
	to[20] = 0;
	while (*p++);	
	/* sms_esm_class  = 0 if single = 0x40 if multiple */
	p++;
	/* sms_protocol_id = 0*/
	p++;
	/* sms_priority_flag = 0*/
	p++;
	/* sms_schedule_delivery_time date/time of delivery schedule (NULL = immadiate)*/
	p++;
	/* sms_validity_period validity duration (NULL = SMSC default) */
	p++;
	/* sms_registered_delivery_flag aka need receipt flag : 0 -> no, 1 -> yes */
	p++; // sms_registered_delivery_flag = *p++;
	/* sms_replace_if_present_flag : 0 -> no, 1 -> yes */
	p++;
	/* sms_data_coding encoding scheme (?) => 0 */
	p++;
	/* sms_sm_default_msg_id if smc defines "canned" message => 0 for specific message */
	p++;	
	/* message length */
	len = ntohl(*(int*) p);
	p += 4;
	/* Message */
	for (i = 0;  i < len && i < 161; i++) message[i] = p[i];

	/* reuse pdu to reply: */
	h->command_len = htonl(sizeof(pdu_t) + 1);
	h->command_id |= htonl(generic_nack);
	h->command_status = 0;
	((char *) h)[sizeof(pdu_t) + 1] = 0;

	smpp_sen(s, (char *) h);

	return 0;
}

char *
smpp_read_pdu(psmpp_t s, char *pdu) {
	char *p;
	size_t /* totlen, */ len, r;

	if (!s) {
		err_error("invalid smpp pointer");
		return NULL;
	} 
	if (!s->cli) {
		err_error("invalid smpp->cli pointer");
		return NULL;
	}
		
	/* initialize varables : */	
	memset(pdu, 0, PDUSIZE);	
	len    = 4;
	// totlen = 0;
	p = pdu;
		
	/* read the 4 bytes of PDU avoiding partial content: */
	while ((r = cli_recv(s->cli, p, len)) < len) {
		len -= r;
		p   += r;
	}	

	if (getenv("SMPPDBG")) err_debug("read %u bytes", pdu_len((ppdu_t) pdu));
	
	/* skip position to second pdu header param: */ 
	p = pdu + 4;
	
	len = pdu_len((ppdu_t) pdu) - 4; 

	/* read the len bytes of PDU avoiding partial content: */
	while ((r = cli_recv(s->cli, p, len)) < len) {
		len -= r;
		p   += r;
	}	

	return pdu;
} 

#if defined(_test_smpp_)

void sms_read_loop() {
	psmpp_t s;
	char pdu[PDUSIZE + 1];
	//smpp_debug(">>> ENTERING READ LOOP:");

	s = smpp_new("SMPP5.mblox.com", 3217);
	smpp_bind(s, smppRECEIVER, "Prologue", "bC2oUzS5");
	
	while (1) {
		ppdu_t p;
		smpp_read_pdu(s, pdu);
		p = (ppdu_t) pdu;
		if (ntohl(p->command_id) != deliver_sm) {
			if (getenv("SMPPDBG")) err_info("Recieved command %08x", ntohl(p->command_id));
			pdu_hexdump(p);
		} else {
			char from[21] , to[21], message[200];
			smpp_sms_parse(s, pdu, from, to, message);
			if (!*from) {
				/* this is a receipt: */
				char *findstr = "^id:([0-9]{10})[ ]* sub:([0-9]{3})[ ]* dlvrd:([0-9]{3})[ ]* submit date:([0-9]{10})[ ]* done date:([0-9]{10})[ ]* stat:([A-Z]{0,7})[ ]* err:([0-9]{3})[ ]* text:(.{0,20})[ ]*$";
				prexp_t r;

				if (!(r = rexp_new(message, findstr, rexp_EXTENDED | rexp_NL_DO_NOT_MATCH)) || !rexp_find(r)) {
					err_error("regular expression error / receipt fields not found");
					if (getenv("SMPPDBG")) err_info("recieved sms receipt for %s containing: \"%s\"", to, message);		
				} else { 
					if (getenv("SMPPDBG")) err_info("message %s sub %s dlvrd %s submit %s done %s status %s err %s msg %s", 
						rexp_sub_get(r, 1), rexp_sub_get(r, 2), rexp_sub_get(r, 3), rexp_sub_get(r, 4),
						rexp_sub_get(r, 5), rexp_sub_get(r, 6), rexp_sub_get(r, 7), rexp_sub_get(r, 8));
				}
				if (r) rexp_destroy(r);

			} else {	
				if (getenv("SMPPDBG")) err_info("recieved sms from %s to %s containing: \"%s\"", from, to, message);		
			}
		}
	}
}

int
main(int n, char *a[]) {
	int i;
	char id[60];
	psmpp_t s;
	err_init(NULL, err_INFO);
	s = smpp_new("SMPP5.mblox.com", 3217);
	smpp_bind(s, smppTRANSMITTER, "Prologue", "bC2oUzS5");

	if (n == 1) {
		smpp_submit_sm(s, "33778822543", "33667110710", "Test sending a short sms...", id);
		if (getenv("SMPPDBG")) err_info("Sent sms to 33667110710 accepted with id %s", id);	
	} else {
		int count = atoi(a[1]);
		if (count > 100000) count = 100000;
		for (i = 0; i < count; i++) {
			char b[15];
			sprintf(b, "336666%06d", i);
			//if (getenv("SMPPDBG")) err_info("send to %s", b);
			smpp_submit_sm(s, "33778822543", b, "Test sending a short sms...", id);
			if (getenv("SMPPDBG")) err_info("sent to %s with id %s", b, id);
		}
	}
	
	smpp_unbind(s);
	smpp_destroy(s);

	sms_read_loop();

	return 0;
}
#endif

