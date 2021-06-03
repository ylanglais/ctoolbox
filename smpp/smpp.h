#ifndef _smpp_h_
#define _smpp_h_

#ifdef __cplusplus
extern "C" {
#endif

#define generic_nack  0x80000000
#include <arpa/inet.h>

#include <smpp_def.h>
#include <pdu.h>

#ifndef _smpp_c_ 

/* 
 * Call back formats and private smpp struct pointer:
 */
typedef int (*f_check_t)(void *, char *);
typedef int (*f_update_t)(void *, char *, int);
typedef void *psmpp_t;
#endif

/*
 * Constructor & destructor like functions:
 */
psmpp_t smpp_new(char *host, int port);
psmpp_t smpp_destroy(psmpp_t s);

/*
 * smpp commands: 
 */
int smpp_bind(psmpp_t smpp, esme_type_e type, char *login, char *passwd);
int smpp_unbind(psmpp_t smpp);
int smpp_outbind(psmpp_t s);
int smpp_query_sm(psmpp_t s);
int smpp_submit_sm(psmpp_t si, char *from, char *to, char *message, char *id);
int smpp_deliver_sm(psmpp_t s);
int smpp_replace_sm(psmpp_t s);
int smpp_cancel_sm(psmpp_t s);
int smpp_enquire_link(psmpp_t s);
int smpp_submit_multi(psmpp_t s);
int smpp_alert_notification(psmpp_t s);
int smpp_data_sm(psmpp_t s);

/*
 *	ESME to SMSC communication:
 */
int smpp_sen(psmpp_t s, char *pdu);
int smpp_rec(psmpp_t s, char *pdu);

/* 
 * primitive to read entire pdus (partial content packects safe:
 */
char *smpp_read_pdu(psmpp_t s, char *pdu);

/*
 * Helper/utils:
 */
char * smpp_cmd_name(int id);
int smpp_sms_parse(psmpp_t s, char *pdu, char *from, char *to, char *message);

#ifdef __cplusplus
}
#endif
#endif

